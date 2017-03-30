#include "socketInclude.h"

/**
 * recieve file from client
 */
void put(HEADER header, int sock) {

	FILE* fp = fopen(header.arg1, "w+");
	if (fp == NULL) {
		printf("Can't create file %s\n", header.arg1);
		return;
	}
	char* buffer = (char*) malloc(sizeof(char) * header.datasize);
	read_n(sock, buffer, header.datasize);
	fwrite(buffer, header.datasize, 1, fp);
	fclose(fp);
}

/**
 * send file to client
 */
void get(HEADER header, int sock) {
	FILE* fp = fopen(header.arg1, "r+");
	if (fp == NULL) { // file do not exsit
		printf("Can't open file %s\n", header.arg1);
		header.datasize = 0;
		header.type = FILE_NO_FOUND;
		write_n(sock, (char*) &header, sizeof(header));
		return;
	} else {

		//get file size
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		header.datasize = size;
		write_n(sock, (char*) &header, sizeof(header));

		//send to buffer
		char* buffer = (char*) malloc(sizeof(char) * size);
		fseek(fp, 0, SEEK_SET);
		fread(buffer, size, 1, fp);
		fclose(fp);
		write_n(sock, buffer, size);
	}
}

/**
 * delete file
 */
void removefile(HEADER header, int sock) {

	char* tmp = (char*) malloc(sizeof(char) * 20);
	sprintf(tmp, "rm %s", header.arg1);

	if (system(tmp) == 0) { // delete file successfully
		header.type = FILE_REMOVED;

	}
	write_n(sock, (char*) &header, sizeof(header));
}

/**
 * list cuurent working directory info ,store to "ltemp.txt",
 * send to client and finally delete it
 */
void list(HEADER header, int sock) {

	system("ls -a>lstemp.txt");
	FILE *fp = fopen("lstemp.txt", "r+");
	if (fp == NULL) {
		printf("Can't open file lstemp.txt\n");
		header.datasize = 0;
		header.type = FILE_NO_FOUND;
		write_n(sock, (char*) &header, sizeof(header));
		return;
	} else {
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		header.datasize = size;
		write_n(sock, (char*) &header, sizeof(header));
		char* buffer = (char*) malloc(sizeof(char) * size);
		fseek(fp, 0, SEEK_SET);
		fread(buffer, size, 1, fp);
		fclose(fp);
		write_n(sock, buffer, size);
		remove("lstemp.txt");

	}

}

int main(int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);
	int socket_fd, new_socket_fd, addr_length, child_pid;
	unsigned long int host_id;
	struct sockaddr_in client_addr, server_addr;
	struct hostent *hp;
	char host_name[256];
	pname = argv[0];
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket ERROR in main");
		exit(1);
	} else {
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);
		//printf("hp:%s\n",hp->h_name);
		if (hp == NULL) {
			printf("gethostbyname ERROR in main: %s does not exist", host_name);
			exit(1);
		} else {
			host_id = *((unsigned long int *) (hp->h_addr_list));
			memcpy(&server_addr.sin_addr, hp->h_addr, hp -> h_length);
			server_addr.sin_port = htons(TCP_PORTNO);
			if (bind(socket_fd, (struct sockaddr *) &server_addr,
					sizeof(server_addr)) < 0) {
				printf("bind ERROR in main");
				exit(1);
			} else {
				addr_length = sizeof(server_addr);
				if (getsockname(socket_fd, (struct sockaddr *) &server_addr,
						&addr_length) < 0) {
					printf("getsockname ERROR in main");
					exit(1);
				} else {
					printf("Parent: Server on port %d\n",
							ntohs(server_addr.sin_port));
					if (listen(socket_fd, 5) < 0) {
						printf("listen ERROR in main");
						exit(1);
					} else {

						for (;;) {
							printf("Parent: Waiting for client\n");
							addr_length = sizeof(client_addr);
							new_socket_fd = accept(socket_fd,
									(struct sockaddr *) &client_addr,
									&addr_length);
							printf("Parent: Client arrived\n");
							if (new_socket_fd < 0) {
								printf("accept ERROR in main");
								exit(1);
							} else {
								if ((child_pid = fork()) < 0) {
									printf("child fork ERROR in main");
									exit(1);
								} else if (child_pid == 0) {
									printf("Child: Fork OK\n");
									close(socket_fd);
									message_echo(new_socket_fd);
									printf("Child: Done\n");
									exit(0);
								} else {
									close(new_socket_fd);
								}
							}
						}
					}
				}
			}
		}
	}
}

void message_echo(int socket_fd) {
	int i, n;
	char line[MAX_LINE_SIZE];

	HEADER header;
	for (;;) {

		memset(&header, 0, sizeof(header));
		read_n(socket_fd, (char*) &header, sizeof(header));

		//  printf("%d\n", header.type);
		if (header.type == LT) {
			list(header, socket_fd);
		} else if (header.type == EXIT) {
			break;
		} else if (header.type == PUT) {
			put(header, socket_fd);
		} else if (header.type == GET) {
			get(header, socket_fd);
		}

		else if (header.type == RMV) {
			removefile(header, socket_fd);
		} else {

		}

	}
}

int read_line(int fd, char *ptr, int line_size) {
	int n, rc;
	char c;

	for (n = 1; n < line_size; n++) {
		if ((rc = read_n(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\n') {
				break;
			}
		} else if (rc == 0) {
			if (n == 1) {
				return (0);
			} else {
				break;
			}
		} else {
			return (-1);
		}
	}
	*ptr = 0;
	return (n);
}

int write_n(int fd, char *ptr, int n_bytes) {
	int n_left, n_written;
	n_left = n_bytes;

	while (n_left > 0) {
		n_written = write(fd, ptr, n_left);
		if (n_written <= 0) {
			return (n_written);
		}
		n_left = n_left - n_written;
		ptr = ptr + n_written;
	}
	return (n_bytes - n_left);
}

int read_n(int fd, char *ptr, int n_bytes) {
	int n_left, n_read;

	n_left = n_bytes;
	while (n_left > 0) {
		n_read = read(fd, ptr, n_left);
		if (n_read < 0) {
			return (n_read);
		} else if (n_read == 0) {
			break;
		}
		n_left = n_left - n_read;
		ptr = ptr + n_read;
	}
	return (n_bytes - n_left);

}
