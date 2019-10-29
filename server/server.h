#ifndef SERVER_H
#define SERVER_H



/*--------------------------------MARCO-----------------------------------*/
#define DEFAULT_PORT 21	
#define CONN 0
#define WELCOME "220 FTP server ready"
#define LISTEN_MAX 300 			//max listen times


enum ClientState {
	START,  		// start connection
    USER,   		// client send USER command
    PASS,   		// clinet send PASS command
    PORT,  		    // PORT mode 
    PASV,           // PASV mode
    STOR,      // call RETR/STOR to fetch data
    RETR
};


struct client_info {
	int client_socket;
	enum ClientState cstate;
	char sentence[8192];
	char client_command[100];
	char client_argument[100];

	int file_socket;
	int port;
	int file_size;

	int file_bytes;
	int file_num;
	int file_restart_position;
};

/*------------------------------FUNCTIONS---------------------------------*/
int server_configure(); //to set port, socket and ip for the server
void configure_root_and_port(int argc, char **argv);
int configure_socket(int *sock, int port);
void send_Info_to_client(char* buffer, int sock);


/*------------------------------VARIABLES---------------------------------*/


// file connection configure
int file_port;
char file_addr[200];
int file_socket;

int client_file_socket;
int server_file_socket;




// server configure
int server_port = DEFAULT_PORT;
int server_socket;     // listen socket

// work directory
char root_path[200];
char cur_path[1000];


// for rename file
char src_file_path[300];
char dst_file_path[300];


char unused_sign[3] = {'\r', '\n', ' '};

#endif