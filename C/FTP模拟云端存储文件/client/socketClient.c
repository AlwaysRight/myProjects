#include "socketInclude.h"


typedef void(*FUNCTION)();
const char* cmds[] = { "listdir", "rmvfile","getfile", "putfile", "stop" };

char line[256];
void fn_list();
void fn_remove();
void fn_put();
void fn_get();
void fn_exit();

int socket_fd;
const char* TOKEN = " \n\r";

FUNCTION functions[] = {fn_list, fn_remove, fn_get,fn_put,fn_exit };




int main (int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
 //  int i;
  // int  trace_flag = 0;
 //  long int pid, net_pid;
	long int pid;
   time_t current_time;
   unsigned long int host_id;
   struct sockaddr_in server_addr;
   struct hostent *hp;
   struct in_addr client_addr;
   char host_name [256];
   current_time = time (NULL);
   pname = argv [0];

   if (argc == 2)
   {
      ;
   }
   else if (argc < 2)
   {
      printf ("argc ERROR in main: Not enough arguments");
      exit (1);
   }

/*
   Create a socket descriptor, specify the type of communication protocol
   (ie. address family and socket type).  If an error occurs, print an error
   message and quit the program.
*/

   if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
   {
      printf ("socket ERROR in main");
      exit (1);
   }

/*
   Establish the remote address and port number of the server.
*/

   memset (&server_addr, 0, sizeof (server_addr));
   server_addr.sin_family = AF_INET;
   hp = gethostbyname (argv [1]);
   if (hp == NULL)
   {
      printf ("gethostbyname ERROR in main: %s does not exist", argv [1]);
      exit (1);
   }
   host_id = *((unsigned long int *) (hp -> h_addr_list));
   memcpy (&server_addr.sin_addr, hp -> h_addr, hp -> h_length);
   server_addr.sin_port = htons (TCP_PORTNO);

/*
   Establish the connection between the client and server processes.  If an
   error occurs, print an error message and quit the program.
*/

   if (connect (socket_fd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0)
   {
      printf ("connect ERROR in main");
      exit (1);
   }

/*
   Establish the local address and process id.
*/

   gethostname (host_name, sizeof (host_name));
   hp = gethostbyname (host_name);
   if (hp == NULL)
   {
      printf ("gethostbyname ERROR in main: %s does not exist", host_name);
      exit (1);
   }
   host_id = *((unsigned long int *) (hp -> h_addr_list));
   memcpy (&host_id, hp -> h_addr, sizeof (host_id));
   pid = getpid ();

/*
   Print informative message about the client.
*/

   client_addr.s_addr = host_id;
   printf ("Socket client started on %s (IP address = %s, PID = %d) at %s", hp -> h_name, inet_ntoa (client_addr), pid, asctime (localtime (&current_time)));

/*
   Re-establish the remote address of the server.
*/

   hp = gethostbyname (argv [1]);
   host_id = *((unsigned long int *) (hp -> h_addr_list));
   memcpy (&host_id, hp -> h_addr, sizeof (host_id));
   current_time = time (NULL);

/*
   Write the process id of the client process to the message stream.  The
   process id should be the first message sent.  It is used to help the server
   identify the origin of messages when concurrent clients are sending
   messages and the trace option is enabled.  If an error occurs, print an
   error message and quit the program.

   net_pid = htonl (pid); 
   if ((i = write_n (socket_fd, (char *) &net_pid, sizeof (net_pid))) != sizeof (net_pid))
   {
      printf ("write_n ERROR in main: Process ID not sent\n");
      exit (1);
   }
*/

/*
   Send messages to the server.
*/



   int cmd_count = 5;
   while (1) {
   		printf("ftp>");
   		fgets(line,MAX_LINE_SIZE,stdin);
   		//printf("%s\n",line);
   		char* cmd = strtok(line, TOKEN);

   		//printf("%s\n",cmd);

   		if (cmd == NULL) {
   			continue;
   		}
   		int i;
   		for (i = 0; i < cmd_count; i++) {
   			if (strcmp(cmd, cmds[i]) == 0) {
   				//printf("%s\n",cmds[i]);
   				(*functions[i])();
   				break;
   			}
   		}
   		if (i == cmd_count) {
   		  printf("Unknown command\n");
   		 // send_message (stdin, socket_fd, trace_flag);
   		}
   	}


/*
   Close the connection between the client and server processes.
*/

   close (socket_fd);

/*
   Re-establish the local address.
*/

   hp = gethostbyname (host_name);
   host_id = *((unsigned long int *) (hp -> h_addr_list));
   memcpy (&host_id, hp -> h_addr, sizeof (host_id));
   current_time = time (NULL);

/*
   Print an informative message about when the client ended.
*/

   client_addr.s_addr = host_id;
   printf ("Socket client on %s (IP address = %s, PID = %d) ended at %s", hp -> h_name, inet_ntoa (client_addr), pid, asctime (localtime (&current_time)));
   exit (0);
}


/**
 * put the file to the server(the filename are stored in heafer.arg1)
 */
void fn_put() {
	HEADER header;
	header.type = PUT;
	char * local_file = strtok(NULL, TOKEN);

	strcpy(header.arg1, local_file);

	//open file
	FILE* fp = fopen(local_file, "r+");
	if (fp == NULL) {
		printf("Can't open file %s\n", local_file);
		return;
	}

	//get size of the file
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	header.datasize = size;


	write_n(socket_fd, (char*)&header, sizeof(header));
	//char* buffer = new char[size];


	// write to buffer
	char* buffer = (char*)malloc(sizeof(char)* size);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, size, 1, fp);
	fclose(fp);
	write_n(socket_fd, buffer, size);
}


/**
 * get the file to the server(the filename are stored in heafer.arg1)
 */
void fn_get() {
	HEADER header;
	header.type = GET;
	char* remote_file = strtok(NULL, TOKEN);

	strcpy(header.arg1, remote_file);


	write_n(socket_fd, (char*)&header, sizeof(header));


	read_n(socket_fd, (char*)&header, sizeof(header));


	// judge if the filename exsit
	if(header.type == FILE_NO_FOUND){


		printf("file do not exist!\n");
		return;
	}
	else{

		//copy to local file named filename from server
		FILE* fp = fopen(remote_file, "w+");
		if (fp == NULL) {
			printf("Can't create file %s", remote_file);
			return;
		}
		char* buffer = (char*)malloc(sizeof(char)* header.datasize);
		read_n(socket_fd, buffer, header.datasize);
		fwrite(buffer, header.datasize, 1, fp);
		fclose(fp);
	}
}

//stop the current client
void fn_exit() {
	HEADER header;
	header.type = EXIT;
	write_n(socket_fd, (char*)&header, sizeof(header));
	close(socket_fd);
	exit(0);
}

//list directory and files of the current directory of server
void fn_list() {
	HEADER header;
	header.type = LT;
	write_n(socket_fd, (char*)&header, sizeof(header));

	read_n(socket_fd, (char*)&header, sizeof(header));

	if(header.type == FILE_NO_FOUND){
		printf("file do not exist!\n");
		return;
	}
	else{
		FILE* fp = fopen("lstemp.txt", "w+");
		if (fp == NULL) {
			printf("Can't create file lstemp.txt");
			return;
		}
		char* buffer = (char*)malloc(sizeof(char)* header.datasize);
		read_n(socket_fd, buffer, header.datasize);
		fwrite(buffer, header.datasize, 1, fp);
		fclose(fp);
	}

}


// remove file named delfile form server
void fn_remove() {
	HEADER header;
	header.type = RMV;
	char* delfile = strtok(NULL, TOKEN);
	strcpy(header.arg1, delfile);
	write_n(socket_fd, (char*)&header, sizeof(header));

	read_n(socket_fd, (char*)&header, sizeof(header));
	if (header.type == FILE_REMOVED) {
		printf("file removed\n");
	}
	else {
		printf("fail to removed\n");
	}
}







/*
   This function is used to send a message to the server.  Messages are read
   from the standard input device and sent to the server until there are no
   more messages to send.
*/

void send_message (FILE *fp, int socket_fd, int trace_flag)
{
	HEADER header;
	header.type = 10;
	write_n(socket_fd, (char*)&header, sizeof(header));
   int i, n, message_no = 1;
   /*
   time_t current_time;
   double elapsed_time;
   time_t request_time, reply_time, net_reply_time; 
   */
   char send_line [MAX_LINE_SIZE], recv_line [MAX_LINE_SIZE];

/*
   Process messages.
*/

   while (fgets (send_line, MAX_LINE_SIZE, fp) != NULL)
   {
      n = strlen (send_line);

/*
   Write characters to the client until there are no more to write.  If an
   error occurs during writing, print an error message.
*/

      if ((i = write_n (socket_fd, send_line, n)) != n)
      {
	 printf ("write_n ERROR in send_message");
	 exit (1);
      }
      else
      {

/*
   Read characters from the server until there are no more to read.  If an
   error occurs during reading, print an error message.
*/

	 n = read_line (socket_fd, recv_line, MAX_LINE_SIZE);
	 if (n < 0)
	 {
	    printf ("read_line ERROR in send_message: Encoded message not received");
	    exit (1);
         }
	 else
	 {
	    fputs (recv_line, stdout);
         }
      }
   }
   if (ferror (fp))
   {
      printf ("message ERROR");
      exit (1);
   }








}
/*
   This function is used to read a message from the server, one character at
   a time, until the newline character is read.  The number of characters
   read, including the newline character, is returned to the calling function.
*/

int read_line (int fd, char *ptr, int line_size)
{
   int n, rc;
   char c;
   for (n = 1; n < line_size; n++)
   {
      if ((rc = read_n (fd, &c, 1)) == 1)
      {
	 *ptr++ = c;
	 if (c == '\n')
	 {
	    break;
         }
      }
      else if (rc == 0)
      {
	 if (n == 1)
	 {
	    return (0);
         }
	 else
	 {
	    break;
         }
      }
      else
      {
	 return (-1);
      }
   }
   *ptr = 0;
   return (n);
}

/*
   This function writes "n" characters to the server.
*/

int write_n (int fd, char *ptr, int n_bytes)
{
   int n_left, n_written;
   n_left = n_bytes;
   while (n_left > 0)
   {
      n_written = write (fd, ptr, n_left);
      if (n_written <= 0)
      {
	 return (n_written);
      }
      n_left = n_left - n_written;
      ptr = ptr + n_written;
   }
   return (n_bytes - n_left);
}



/*
int Write(int fd, void* buffer, int size) {
	int ret = write(fd, buffer, size);
	if (ret < 0) {
		perror("write error!\n");
	}
	return ret;
}

*/


/*
   This function reads "n" characters from the server.
*/

int read_n (int fd, char *ptr, int n_bytes)
{
   int n_left, n_read;
   n_left = n_bytes;
   while (n_left > 0)
   {
      n_read = read (fd, ptr, n_left);
      if (n_read < 0)
      {
	 return (n_read);
      }
      else if (n_read == 0)
      {
	 break;
      }
      n_left = n_left - n_read;
      ptr = ptr + n_read;
   }
   return (n_bytes - n_left);
}
