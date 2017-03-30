#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/file.h>

#define TCP_PORTNO 4259
#define MAX_LINE_SIZE 255 


#define LT	1
#define RMV	2
#define PUT	3
#define GET	4
#define EXIT 5

#define FILE_REMOVED -1
#define FILE_UNREMOVED -2
#define FILE_NO_FOUND -3


typedef struct {
	int type;
	int datasize;
	char arg1[256];
} HEADER;



char *pname;

void send_message (FILE *fp, int socket_fd, int trace_flag);
int read_line (int fd, char *ptr, int line_size);
int write_n (int fd, char *ptr, int n_bytes);
int read_n (int fd, char *ptr, int n_bytes);
void message_echo (int socket_fd);
void display_error (int error_code);
