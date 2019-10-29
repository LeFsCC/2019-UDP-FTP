#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <sys/stat.h>
#include <netdb.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <fcntl.h>

#include  "server.h"
#include  "utils.h"


int server_configure(int argc, char **argv)	{
	configure_root_and_port(argc, argv);
	configure_socket(&server_socket, server_port);
	return 1;
}

void get_client_msg(struct client_info *client) {
	char  buf[8192];
	memset(buf, 0, sizeof(buf));
	recv(client->client_socket, buf, 8192, 0);
	int length = strlen(buf);

	for(int i = length-1; i>0; i--) {
		for (int j = 0;j<3;j++) {
			if (buf[i] == unused_sign[j]) {
				buf[i] = '\0';
			} else {
				break;
			}
		}
	}
	memset(client->sentence, 0, sizeof(client->sentence));
	strcpy(client->sentence, buf);
	return;
}

void extract_command_and_argument(struct client_info *client) {
	memset(client->client_command, 0, sizeof(client->client_command));
	memset(client->client_argument, 0, sizeof(client->client_argument));
	for(int i = strlen(client->sentence)-1; i>0; i--) {
		if(client->sentence[i] == '\r' || client->sentence[i] == '\n') {
			client->sentence[i] = '\0';
		}
	}
	sscanf(client->sentence, "%s %s", client->client_command, client->client_argument);
}

void configure_root_and_port(int argc, char **argv) {

	server_port = DEFAULT_PORT;
	strcpy(root_path, "/tmp/");
	// get root path from input
	for(int i = 0; i< argc; i++) {
		if(strcmp(argv[i], "-root") == 0) {
			strcpy(root_path, argv[i+1]);
			break;
		}
	}
	// get port from input
	for(int i = 0; i< argc; i++) {
		if(strcmp(argv[i], "-port") == 0) {
			server_port = atoi(argv[i+1]);
			break;
		}
	}
	// initial : current path equals to root path
	strcpy(cur_path, root_path);
	if(root_path[strlen(root_path)-1] != '/') {
		strcat(cur_path, "/");
	}
}
int configure_socket(int *sock, int port) {

	struct sockaddr_in addr;
	//创建socket
	if ((*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	//设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"127.0.0.1"

	//将本机的ip和port与socket绑定
	if (bind(*sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}

	//开始监听socket
	if (listen(*sock, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	return 1;
}

void send_Info_to_client(char* buffer, int m_sock) {
	char temp[1024];
	strcpy(temp, buffer);
	strcat(temp, "\r\n");
	send(m_sock, temp, strlen(temp), 0);
}

void listen_client(struct client_info *client) {

	send_Info_to_client(WELCOME, client->client_socket);
	char buf[8192];
	DIR* dir;
	client->file_bytes = 0;
	client->file_num = 0;
	client->cstate = START;
	client->file_restart_position = 0;

	while(1) {
			get_client_msg(client);
			extract_command_and_argument(client);
			if(!check_command(client->client_command)) {
				strcpy(buf, "503 Bad sequence of commands.");
    			send_Info_to_client(buf, client->client_socket);
				continue;
			}
			printf("client : %s %s\r\n", client->client_command, client->client_argument);
			// user login
			if (!strcmp(client->client_command, "QUIT")) {
				sprintf(buf, "221 You have transferred %d bytes in %d ﬁles.", client->file_bytes, client->file_num);
				strcat(buf, "221 Thank you for using the FTP service.\r\n Goodbye! \r\n");
    			send_Info_to_client(buf, client->client_socket);
    			break;
			} else if(!strcmp(client->client_command, "USER")) {
				memset(buf, 0, sizeof(buf));
				// check username
				if(strcmp(client->client_argument, "anonymous") != 0) {
					strcpy(buf, "430 please login with 'anonymous' first");
					send_Info_to_client(buf, client->client_socket);
				} else {
    				strcpy(buf, "331 Guest login ok,send your complete e-mail address as password.");
    				client->cstate = USER;
    				send_Info_to_client(buf, client->client_socket);
				}
			} 
			// client pass an password to server
			else if(!strcmp(client->client_command, "PASS")) {
				memset(buf, 0, sizeof(buf));
				client->cstate = PASS;
				strcpy(buf, "230 Login successful.");
				send_Info_to_client(buf, client->client_socket);
			} 
			// client determine the server operating system
			else if(!strcmp(client->client_command, "SYST")) {
				memset(buf, 0, sizeof(buf));
				if(client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					strcpy(buf, "215 UNIX Type: L8");
					send_Info_to_client(buf, client->client_socket);

				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// type operation
			else if (!strcmp(client->client_command, "TYPE")) {
				memset(buf, 0, sizeof(buf));
				if(client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					if(strlen(client->client_argument) == 0) {
						strcpy(buf, "530 TYPE command must have argument!");
						send_Info_to_client(buf, client->client_socket);
					} else {
						strcpy(buf, "200 Type set to I.");
						send_Info_to_client(buf, client->client_socket);
					}
					
				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// make file directory
			else if (!strcmp(client->client_command, "MKD")) {
				if (client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {

    				strcat_filefold_path(cur_path, client->client_argument, buf);
    				int i = mkdir(buf, 0700);
    				char temp[1000];
    				memset(temp, 0, sizeof(temp));
    				strcpy(temp, buf);
    				// file created
    				if(i == 0) {
    					sprintf(buf, "257 %s directory created", temp);
    				} else {
    					sprintf(buf, "550 %s directory already exists", temp);
    				}
    				send_Info_to_client(buf, client->client_socket);	
				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
				
			}
			// change work directory
			else if (!strcmp(client->client_command, "CWD")) {
				memset(buf, 0, sizeof(buf));
				if(client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					dir = opendir(client->client_argument);
					if(dir) {
						closedir(dir);
						strcpy(cur_path, client->client_argument);
						strcat(cur_path, "/");
						sprintf(buf, "250 changed to %s", cur_path);
						send_Info_to_client(buf, client->client_socket);
					} else {
						sprintf(buf, "550 changed failed, working path stays still %s", cur_path);
						send_Info_to_client(buf, client->client_socket);
					}
				}
				else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// print current directory
			else if(!strcmp(client->client_command, "PWD")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					sprintf(buf, "257 %s is the current path", cur_path);
					send_Info_to_client(buf, client->client_socket);
				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// In port mode, client open a port to wait for data transfer
			else if(!strcmp(client->client_command, "PORT")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					memset(file_addr, 0, sizeof(file_addr));
					file_port = 0;
					if(parse_haddr_and_port(client->client_argument, file_addr, &file_port)) {
						client->cstate = PORT;
						strcpy(buf, "200 PORT successful");
						send_Info_to_client(buf, client->client_socket);
					} else {
						strcpy(buf, "501 Invaild argument");
						send_Info_to_client(buf, client->client_socket);
					}
				}
				else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// in passive mode, server open a port to wait for data transfer
			else if(!strcmp(client->client_command, "PASV")) {
				memset(buf, 0, sizeof(buf));
				get_random_port(&file_port);
				get_local_ip(file_addr);
				if(is_vaild_ip(file_addr)) {
					int if_configure = 0;
					if_configure = configure_socket(&server_file_socket, file_port);
					client->cstate = PASV;
					char temp_addr[100];
					char temp_port[100];
					memset(temp_addr, 0, sizeof(temp_addr));
					memset(temp_port, 0, sizeof(temp_port));
					
					decorate_addr_port(file_addr, temp_addr, file_port, temp_port);
					if (if_configure == 1) {
						sprintf(buf, "227 Entering Passive Mode(%s,%s)", temp_addr, temp_port);
					} else {
						sprintf(buf, "530 failed to enter passive mode(%s,%s)", temp_addr, temp_port);
					}
					
					send_Info_to_client(buf, client->client_socket);
				} else {
					strcpy(buf, "530 failed to enter passive mode");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// romove file directory
			else if(!strcmp(client->client_command, "RMD")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
    				strcat_filefold_path(cur_path, client->client_argument, buf);

    				int i = rmdir(buf);
    				char temp[1000];
    				memset(temp, 0, sizeof(temp));
    				strcpy(temp, buf);
    				// file created
    				if(i == 0) {
    					sprintf(buf, "250 %s directory removed", temp);
    				} else {
    					sprintf(buf, "550 %s directory not found", temp);
    				}
    				send_Info_to_client(buf, client->client_socket);	
				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// rename from, indicate the old file name, should be followed by "RNTO"
			else if(!strcmp(client->client_command, "RNFR")) {
				memset(buf, 0, sizeof(buf));
				if(client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					if (strlen(client->client_argument) == 0) {
						strcpy(buf, "501 Invaild argument, file name should not be empty!");
						send_Info_to_client(buf, client->client_socket);
					} else {
						strcpy(src_file_path, cur_path);
						strcat(src_file_path, client->client_argument);
						// import system control command to pretend to rename file but actually detect its xistence
						int i = rename(src_file_path, src_file_path);

						// rename prepare successfully
						if (i == 0) {
							strcpy(buf, "350 please specified destination file name");
							send_Info_to_client(buf, client->client_socket);
						} else {
							strcpy(buf, "550 File name not allowed.");
							send_Info_to_client(buf, client->client_socket);
						}
					}
				} else {
					strcpy(buf, "530 please login and pass the password");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// rename to, indicate new file name, should follow command "RNFR"
			else if(!strcmp(client->client_command, "RNTO")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PASS || client->cstate == PORT || client->cstate == PASV) {
					if(strlen(client->client_argument) == 0) {
						strcpy(buf, "501 Invaild argument, file name should not be empty!");
						send_Info_to_client(buf, client->client_socket);
					} else {
						strcpy(dst_file_path, cur_path);
						strcat(dst_file_path, client->client_argument);
						int i = rename(src_file_path,dst_file_path);
						if (i == 0) {
							sprintf(buf, "250 file rename from %s to %s", src_file_path, dst_file_path);
							send_Info_to_client(buf, client->client_socket);
						} else {
							sprintf(buf, "502 failed to rename file %s to %s", src_file_path, dst_file_path);
							send_Info_to_client(buf, client->client_socket);
						}
					}
				} else {
					strcpy(buf, "502 should asign source file name first");
					send_Info_to_client(buf, client->client_socket);
				}
			}
			// file transfer
			else if(!strcmp(client->client_command, "RETR")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PORT) {
					// server connect client
					
					connect_client(&client_file_socket, file_addr, &file_port);

					strcpy(buf, cur_path);
					strcat(buf, client->client_argument);
					int size = filesize(buf);

					if (size <= 0) {
						char temp[300];
						sprintf(temp, "550 file %s not found", client->client_argument);
						send_Info_to_client(temp, client->client_socket);
					} else {
						char temp_2[300];
						sprintf(temp_2, "150 Opening BINARY mode data connection for %s(%d)", client->client_argument, size);
						send_Info_to_client(temp_2, client->client_socket);

						if(push_file(&client_file_socket, buf, client->file_restart_position)) {
							char temp[300];
							memset(temp, 0, sizeof(temp));
							int m_size = size >> 20;
							int k_size = (size - (m_size << 20)) >> 10;

							client->file_bytes += size;
							client->file_num += 1;
							close(client_file_socket);
							sprintf(temp, "226 file %s transfer completed, %dM %dK data transfered", client->client_argument, m_size, k_size);
							send_Info_to_client(temp, client->client_socket);
						} else {
							char temp[300];
							strcpy(temp, "550 file not found");
							send_Info_to_client(temp, client->client_socket);
						}
					}
					close(client_file_socket);
				} else if(client->cstate == PASV) {
					// passive data transfer mode
					// blocking function, wait for client connection

					if ((client_file_socket = accept(server_file_socket, NULL, NULL)) == -1) {
			            //printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			            continue;
			        }
			        // server connect client
					strcpy(buf, cur_path);
					strcat(buf, client->client_argument);
					int size = filesize(buf);
					if (size <= 0) {
						char temp[300];
						sprintf(temp, "550 file %s not found", client->client_argument);
						send_Info_to_client(temp, client->client_socket);
					} else {
						char temp_2[300];
						sprintf(temp_2, "150 Opening BINARY mode data connection for %s(%d)", client->client_argument, size);
						send_Info_to_client(temp_2, client->client_socket);
						if(push_file(&client_file_socket, buf, client->file_restart_position)) {
							char temp[300];
							memset(temp, 0, sizeof(temp));
							int m_size = size >> 20;
							int k_size = (size - (m_size << 20)) >> 10;
							close(client_file_socket);
							close(server_file_socket);

							client->file_bytes += size;
							client->file_num += 1;
							client_file_socket = -1;
							server_file_socket = -1;
							if(client_file_socket == -1 && server_file_socket == -1) {
								sprintf(temp, "226 file %s transfer completed, %dM %dK data transfered", client->client_argument, m_size, k_size);
								send_Info_to_client(temp, client->client_socket);
							} 
						} else {
							char temp[300];
							strcpy(temp, "550 file not found");
							send_Info_to_client(temp, client->client_socket);
						}
					}
					close(client_file_socket);
					close(server_file_socket);
				} else {
					strcpy(buf, "503 require PORT/PASV mode");
					send_Info_to_client(buf, client->client_socket);
				}
				client->cstate = PASS;
				client->file_restart_position = 0;
			}
			// display all the 
			else if (!strcmp(client->client_command, "LIST")) {
				memset(buf, 0, sizeof(buf));
				if (client->cstate == PORT || client->cstate == PASV) {
					

					if(client->cstate == PORT) {
						connect_client(&client_file_socket, file_addr, &file_port);
						strcpy(buf, "150 Opening BINARY mode data connection for");
						send_Info_to_client(buf, client->client_socket);

						get_directory(cur_path);
						memset(buf, 0, sizeof(buf));
						strcpy(buf, "file_list.txt");
						push_file(&client_file_socket, buf, client->file_restart_position);
						close(client_file_socket);
						strcpy(buf, "226 file list transfered");
						send_Info_to_client(buf, client->client_socket);
					}
					else if (client->cstate == PASV) {
						// passive data transfer mode
						// blocking function, wait for client connection
						if ((client_file_socket = accept(server_file_socket, NULL, NULL)) == -1) {
				           // printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				            continue;
				        }
				        strcpy(buf, "150 Opening BINARY mode data connection");
						send_Info_to_client(buf, client->client_socket);

				        get_directory(cur_path);
				        memset(buf, 0, sizeof(buf));
				        strcpy(buf, "file_list.txt");
				        push_file(&client_file_socket, buf, client->file_restart_position);
				        close(client_file_socket);
				        close(server_file_socket);
				        strcpy(buf, "226 file list transfered");
						send_Info_to_client(buf, client->client_socket);
					}
				} else {
					strcpy(buf, "503 require PORT/PASV mode");
					send_Info_to_client(buf, client->client_socket);
				}
				client->cstate = PASS;
				client->file_restart_position = 0;
			}
			else if(!strcmp(client->client_command, "STOR")) {
				if (client->cstate ==  PORT || client->cstate == PASV) {
					

					if (client->cstate == PORT) {
						// server connect client
						strcpy(buf, cur_path);
						strcat(buf, client->client_argument);
						connect_client(&client_file_socket, file_addr, &file_port);
						char mtemp[300];
						memset(mtemp, 0 , sizeof(mtemp));
						sprintf(mtemp, "150 Opening BINARY mode data connection for %s", client->client_argument);
						send_Info_to_client(mtemp, client->client_socket);

						if(clone_file(&client_file_socket, buf)) {
							char temp[300];
							memset(temp, 0, sizeof(temp));
							int size = filesize(buf);
							int m_size = size >> 20;
							int k_size = (size - (m_size << 20)) >> 10;

							client->file_bytes += size;
							client->file_num += 1;
							close(client_file_socket);
							sprintf(temp, "226 file %s transfer completed, %dM %dK data transfered", client->client_argument, m_size, k_size);
							send_Info_to_client(temp, client->client_socket);
						} else {
							close(client_file_socket);
							char temp[300];
							strcpy(temp, "550 file not found");
							send_Info_to_client(temp, client->client_socket);
						}
						
					}
					else if(client->cstate == PASV) {
						// passive data transfer mode
						// blocking function, wait for client connection
						if ((client_file_socket = accept(server_file_socket, NULL, NULL)) == -1) {
				           // printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				            continue;
				        }
				        sprintf(buf, "150 Opening BINARY mode data connection for %s", client->client_argument);
						send_Info_to_client(buf, client->client_socket);
						memset(buf, 0, sizeof(buf));

				        // server connect client
						strcpy(buf, cur_path);
						strcat(buf, client->client_argument);
						if(clone_file(&client_file_socket, buf)) {
							char temp[300];
							memset(temp, 0, sizeof(temp));
							int size = filesize(buf);
							int m_size = size >> 20;
							int k_size = (size - (m_size << 20)) >> 10;

							client->file_bytes += size;
							client->file_num += 1;
							close(client_file_socket);
							close(server_file_socket);
							sprintf(temp, "226 file %s transfer completed, %dM %dK data transfered", client->client_argument, m_size, k_size);
							send_Info_to_client(temp, client->client_socket);
						} else {
							close(client_file_socket);
							close(server_file_socket);
							char temp[300];
							strcpy(temp, "550 file not found");
							send_Info_to_client(temp, client->client_socket);
						}
						
					}
				}
				client->cstate = PASS;
			}
			else if(!strcmp(client->client_command, "REST")) {
				memset(buf, 0, sizeof(buf));
				client->file_restart_position = atoi(client->client_argument);
				sprintf(buf, "350 restart retr or stor command from %s", client->client_argument);
				send_Info_to_client(buf, client->client_socket);
			}
			else {
				continue;
			}
		} 
}


int main(int argc, char **argv) {
	// configure server port and socket
	server_configure(argc, argv);
	pid_t pid;
	// listen
	while(1) {
		struct client_info client;
		// accept is a blocking function, it returns a connecting socket, 
		// and maintain its listening socket
		if ((client.client_socket = accept(server_socket, NULL, NULL)) == -1) {
            // printf("Error accept(1): %s(%d)\n", strerror(errno), errno);
            break;
        }
        else {
        	pid = fork();   // satablish child process
        	if(pid == 0) {
        		client.cstate = START;
        		close(server_socket);
        		listen_client(&client);
        		close(client.client_socket);
        	} else if (pid < 0) {
        		// perror("Error failed forking processes");
        	}
        	close(client.client_socket);
        }
	}
	close(server_socket);
	return 1;
}