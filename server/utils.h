#ifndef UTILS_H
#define UTILS_H



// strcat current file path and new file path
void strcat_filefold_path(char *curpath, char *nextpath, char* bufpath);

// extract haddr and port from argument
int parse_haddr_and_port(char *buf, char *haddr, int *port);

// for pasv mode, server will establish new data link for file transfer
void get_local_ip(char *buf);
void get_random_port(int *port);
int is_vaild_ip(char *buf);
int connect_client(int *csock, char *caddr, int *cport);
int filesize(char *file_path);
int push_file(int *sock, char *file_path, int offset);

int clone_file(int *sock, char *file_path);
void decorate_addr_port(char *addr, char *temp_addr, int port, char *temp_port);
int get_directory(char *cur_path);
int check_command(char *buf);


#endif