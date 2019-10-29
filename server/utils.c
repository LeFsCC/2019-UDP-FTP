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

#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>

#include <sys/stat.h>

#include "utils.h"

#define BUF_SIZE 8192



void strcat_filefold_path(char *curpath, char *nextpath, char* bufpath) {
	strcpy(bufpath, curpath);
	int len = strlen(nextpath);
	for(int i = len-1; i>0; i--) {
		if (nextpath[i] == '\r' || nextpath[i] == '\n') {
			nextpath[i] = '\0';
		} else {
			break;
		}
	}
	strcat(bufpath, nextpath);
}

int parse_haddr_and_port(char *buf, char *haddr, int *port) {
	int h1 = -1, h2=-1, h3=-1, h4=-1, p1=-1, p2=-1;
	sscanf(buf, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
	if ((h1 == -1 || h2==-1 || h3==-1 || h4==-1 || p1==-1 || p2==-1)) {
		return 0;
	} else {
		sprintf(haddr, "%d.%d.%d.%d", h1, h2, h3, h4);
		p1 = p1 << 8;
		*port = p1 + p2;
		return 1; 
	}
}

void get_local_ip(char *buf) {
	char name[65];
	struct hostent* pHost;
	struct hostent hs;
	int i, j;
	int cnt = 0;
	char *ips[8];

	gethostname(name, sizeof(name));
	pHost = &hs;
	pHost = gethostbyname(name);

	for (i = 0; pHost != 0 && pHost->h_addr_list[i] != 0; i++) {
		char ip[16]; ip[0] = 0;
		for (j = 0; j < pHost->h_length; j++) {
			char tmp[16];
			if (j > 0) {
				strcat(ip, ".");
			}
			sprintf(tmp, "%u",
				(unsigned int)((unsigned char*)pHost->h_addr_list[i])[j]);
			strcat(ip, tmp);
		}
		ips[cnt] = (char*)malloc(64);
		strcpy(ips[cnt++], ip);
	}
	for (int i = 0; i < 8; i++) {
		if (ips[i] != NULL) {
			if(is_vaild_ip(ips[i])) {
				strcpy(buf, ips[i]);
				break;
			}
		}
	}

}

int is_vaild_ip(char *buf) {
	int h1 = -1, h2=-1, h3=-1, h4=-1;
	sscanf(buf, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
	if (h1 == -1 || h2==-1 || h3==-1 || h4==-1) {
		return 0;
	} else {
		return 1;
	}
}
void get_random_port(int *port) {
	// [20000, 65535]
	*port = rand() % 45536 + 20000;
}
// in port mode, server connects to client


int connect_client(int *csock, char *caddr, int *cport) {
	struct sockaddr_in addr;
    //创建socket
	if ((*csock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		//printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}

	//设置目标主机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(*cport);

	if (inet_pton(AF_INET, caddr, &addr.sin_addr) <= 0) {			//转换ip地址:点分十进制-->二进制
		//printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}

	//连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(*csock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		//printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 0;
	}
	return 1;
}

int filesize(char *filename) {
	FILE *f = fopen(filename, "rb");
    if(!f){
        return 0;
    }
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    return size;
}


int push_file(int *sock, char *file_path, int offset) {

	FILE* fp = fopen(file_path, "rb");

	if (!fp) {
		//printf("cannot open file\r\n");
		return 0;
	}

	if(offset > 0) {
		fseek(fp, offset, SEEK_SET);
	}

	int nCount = 2;
	char buffer[BUF_SIZE];
	memset(buffer, 0, sizeof(buffer));

	// use bit stream to send, 8192 bits 
	while( nCount > 0 ) {
		nCount = fread(buffer, 1, BUF_SIZE, fp);
		send(*sock, buffer, nCount, 0);
	}
	return 1;
}


int clone_file(int *sock, char *file_path) {
	FILE* f = fopen(file_path, "wb");
	if(!f) {
		//printf("file cannot create");
		return 0;
	}
    int n = 2;
    char buffer[BUF_SIZE];
 	while(n > 0) {
 		n = recv(*sock, buffer, BUF_SIZE, 0);
        fwrite(buffer, 1, n, f);
 	}
    fclose(f);
    return 1;
}

// put file list in a text file and send it
int get_directory(char *cur_path) {
	// create a file_list container first
	char buf[200];
	memset(buf, 0, sizeof(buf));
    sprintf(buf, "ls -la %s | tail -n+2 > file_list.txt", cur_path);
    int i = system(buf);
    if(i < 0){
    	//printf("failed to create list\r\n");
        return 0;
    }
    return 1;
}

void decorate_addr_port(char *addr, char *temp_addr, int port, char *temp_port) {

	strcpy(temp_addr, addr);
	for(int i = 0; i < strlen(addr); i++) {
		if(temp_addr[i] =='.') {
			temp_addr[i] = ',';
		}
	}
	int p1 = port / 256;
	int p2 = port % 256;
	sprintf(temp_port, "%d,%d", p1, p2);
}

int check_command(char *cmd) {
	int t1 = strcmp(cmd, "USER");
	int t2 = strcmp(cmd, "PASS");
	int t3 = strcmp(cmd, "TYPE");
	int t4 = strcmp(cmd, "MKD");
	int t5 = strcmp(cmd, "RNFR");
	int t6 = strcmp(cmd, "RMD");
	int t7 = strcmp(cmd, "RNTO");
	int t8 = strcmp(cmd, "PWD");
	int t9 = strcmp(cmd, "CWD");
	int t10 = strcmp(cmd, "QUIT");
	int t11 = strcmp(cmd, "PORT");
	int t12 = strcmp(cmd, "PASV");
	int t13 = strcmp(cmd, "RETR");
	int t14 = strcmp(cmd, "LIST");
	int t15 = strcmp(cmd, "STOR");
	int t16 = strcmp(cmd, "SYST");
	int t17 = strcmp(cmd, "REST");
	int t18 = strcmp(cmd, "ABOR");
	if(!(t1 && t2 && t3 && t4 && t5 && t6 && t7 && t8 && t9 && t10 && t11 && t12 && t13 && t14 && t15 && t16 && t17 && t18)) {
		return 1;
	} else {
		return 0;
	}
}