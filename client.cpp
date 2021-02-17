#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h> 

#define AWSPORT "24595"   //aws TCP port
#define HOST "localhost"

using namespace std;
//code refrence: beej
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//code refrence: beej
int main(int argc, char* argv[]){	

	int socket_fd = 0;
	char Map_ID = *(argv[1]);
    char Source_Vertex_Index = atoi(argv[2]);
    char File_Size = atoi(argv[3]);

	struct addrinfo hints, *server_info, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int res = getaddrinfo(HOST, AWSPORT, &hints, &server_info);

	if ( res!= 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		return 1;
	}

	for (p = server_info; p != NULL; p = p->ai_next) {
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))==-1) {
			perror("client: socket");
			continue;
		}

		if (connect(socket_fd, p->ai_addr, p->ai_addrlen)==-1) {
			close(socket_fd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect. \n");
		exit(0);
		return 2;
	}
	char s[INET6_ADDRSTRLEN];
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(server_info); // all done with this structure
	printf("The client is up and running. \n");


	send(socket_fd, &Map_ID, sizeof(Map_ID), 0);
	send(socket_fd, &Source_Vertex_Index, sizeof(Source_Vertex_Index), 0);
	send(socket_fd, &File_Size, sizeof(File_Size), 0);

	printf("The client has sent query to AWS using TCP over port %s: start vertex %d; map %c; file size %d.\n", AWSPORT, Source_Vertex_Index, Map_ID, File_Size);

    close(socket_fd);

    return 0;
}
