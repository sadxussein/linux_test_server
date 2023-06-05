#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>	// system calls data types
#include <sys/socket.h>	// socket structures
#include <netinet/in.h>	// internet domain structures

void error(char *msg) {
	perror(msg);	// system error message
	exit(1);	// exit with general error code
}

void handle_client(int sock) {
	int n;	// return value for read() and write() syscall; number of chars read or written
	char buffer[1500];	// server reads chars from socket into this buffer
	
	bzero(buffer, 1500);	// filling buffer with zeroes
	
	n = read(sock, buffer, 1500);	// read() operates with new socket file descriptor returned by accept(), not the original; original sockfd still only awaits connections
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s", buffer);	// output clients message
	
	n = write(sock, "Message recieved by server", 26);	// since connection is established, server and client can communicate indefinitely
	if (n < 0) error("ERROR writing to socket");
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd;	// file descriptors, to which sockets connects
	int portno;	// port number, on which server accepts connection
	int clilen;	// client address size, required by accept() system call	
	struct sockaddr_in serv_addr, cli_addr;	// server and client addresses
	
	if (argc < 2) {	// user should provide port number in application argument for server to run
		fprintf(stderr, "ERROR, no port provided");
		exit(1);	// exit with general error code
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// passing socket to file descriptor; 2nd arg = SOCK_STREAM | SOCK_DGRAM (TCP | UDP); 3rd arg should remain 0, system will choose the most appropriate protocol; if everything is hunky-dory socket() returns non-negative small integer
	if (sockfd < 0) error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));	// initialize server address variable with zeroes
	portno = atoi(argv[1]);	// simple convertion of passed by user port argument from string to int
	
	serv_addr.sin_family = AF_INET;	// symbolic constant, representing internet type of socket connection
	serv_addr.sin_port = htons(portno);	// htons() converts byte order from host to network representation (from Little endian to Big endian)
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// symbolic constant, which sets servers IP address
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");	// binding socket file descriptor to an address
	
	listen(sockfd, 5);	// system call, which (surprisingly) listens socket for connections; second arg is the size of the backlog queue, number of connections that can be waiting while the process is handling a particular connection; 5 is the max size permitted by most systems
	
	clilen = sizeof(cli_addr);
	while (true) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	// when accept recieves connection from client it creates new file descriptor based on original sockfd, which now handles serves-client communication; original sockfd listens for new connections, it does not participates in server-client communication directly; cli_addr and clilen are recieved from client
		if (newsockfd < 0) error ("ERROR on accept()");	// if things go south (which is unlikely, since accept() works properly 99% of the time) return error
		pid = fork();	// forking the process to allow multiple connections
		if (pid < 0) error("ERROR on fork()");
		if (pid == 0) {	
			close(sockfd);	// newly created child process closes sockfd, it only requires newsockfd to communicate with client
			handle_client(newsockfd);	// passing this socket as argument, working with client
			exit(0);	// when work is done, close the processes
		} else close(newsockfd);
	}		
	
	return 0;	// terminating main as specified by ascii standart
}