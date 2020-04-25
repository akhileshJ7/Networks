// Simple TCP echo server

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>  
#include <string.h>	
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  

void sendFile(int connection);

// Max message to echo
#define MAX_MESSAGE	500

/* server main routine */

int main(int argc, char** argv) {

	// locals
	unsigned short port = 80; // default port
	int sock; // socket descriptor

	// Was help requested?  Print usage statement
	if (argc > 1 && ((!strcmp(argv[1], "-?")) || (!strcmp(argv[1], "-h")))) {
		printf(
				"\nUsage: tcpechoserver [-p port] port is the requested \
 port that the server monitors.  If no port is provided, the server \
 listens on port 22222.\n\n");
		exit(0);
	}

	// get the port from ARGV
	if (argc > 1 && !strcmp(argv[1], "-p")) {
		if (sscanf(argv[2], "%hu", &port) != 1) {
			perror("Error parsing port option");
			exit(0);
		}
	}

	// ready to go
	printf("tcp echo server configuring on port: %d\n", port);

	// for TCP, we want IP protocol domain (PF_INET)
	// and TCP transport type (SOCK_STREAM)
	// no alternate protocol - 0, since we have already specified IP

	if ((sock = socket( PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error on socket creation");
		exit(1);
	}

	// lose the pesky "Address already in use" error message
	int yes = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	// establish address - this is the server and will
	// only be listening on the specified port
	struct sockaddr_in sock_address;

	// address family is AF_INET
	// our IP address is INADDR_ANY (any of our IP addresses)
	// the port number is per default or option above

	sock_address.sin_family = AF_INET;
	sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_address.sin_port = htons(port);

	// we must now bind the socket descriptor to the address info
	if (bind(sock, (struct sockaddr *) &sock_address, sizeof(sock_address))
			< 0) {
		perror("Problem binding");
		exit(-1);
	}

	// extra step to TCP - listen on the port for a connection
	// willing to queue 5 connection requests
	if (listen(sock, 5) < 0) {
		perror("Error calling listen()");
		exit(-1);
	}


	int connection;
	int pid;

	while (1) {

		// hang in accept and wait for connection
		printf("====Waiting====\n");
		connection = accept(sock, NULL, NULL);
		if ( connection < 0) {
			perror("Error calling accept");
			exit(-1);
		}

		// Child process handles file sending; parent waits for more connections
		pid = fork();
		if(pid < 0){
			perror("error on fork");
		}

		// Child process sends file
		if(pid == 0){
			close(sock);
			sendFile(connection);
			exit(0);
		}

		// Parent closes connection so it can open new ones
		else{
			close(connection);
		}
	}

	// will never get here
	return (0);
}
void sendFile(int connection){

	char* readBuffer = calloc(MAX_MESSAGE, sizeof(char));
	char* writeBuffer = calloc(MAX_MESSAGE, sizeof(char));
	int bytes_read;
	int echoed;
	char* file;

	readBuffer[0] = '\0'; 	// guarantee a null here to break out on a disconect
	writeBuffer[0] = '\0';

	// Read HTTP request from client
	bytes_read = read(connection,readBuffer,MAX_MESSAGE-1);

	char delim[] = " ";

	// String is parsed to find the requested page
	char *request = strtok(readBuffer, delim);
	// Has to parse past the GET
	request = strtok(NULL, delim);

	// Index file will be sent if it was requested (or if default)
	if(strcmp(request, "/") == 0 || strcmp(request, "/index.html") ==0){
		file = "index.html";
	}
	
	//Picture file will be sent if there was request for image
	else if(strcmp(request, "/pic_trulli.jpg") ==0)
	{
		// send header first for the picture
		char imageheader[] = "HTTP/1.1 200 Ok\r\n"
			"Content-Type: image/jpeg\r\n\r\n";
		// write header to the connection
		write(connection, imageheader, sizeof(imageheader) - 1);
		file = "pic_trulli.jpg";
	}


	// 404 page will be sent if a different resource was requested
	else{
		file = "404.html";
	}

	// Open the given file to write to client
	FILE *fptr;
	if ((fptr = fopen(file, "rb")) == NULL) {
		printf("Error! opening file");
		exit(1);
	}

	// Keep sending until end of file
	while (!feof(fptr)) {
		bytes_read = fread(writeBuffer, 1, 25,fptr);

		// Break from loop if nothing was read
		if(bytes_read == 0){
			break;
		}

		// Send buffer to client
		if ((echoed = write(connection, writeBuffer, bytes_read)) < 0) {
			perror("Error sending echo");
			exit(-1);
		}
	}
	//fclose(rFile);
	// Close file being read
	fclose(fptr);

	// Close connection with client after file has been sent
	close(connection);

}
