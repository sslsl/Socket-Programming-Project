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
#include<iostream>
#include<iomanip>

#include "def.h"
// #define AWSPORT "24595"   //aws TCP port
// #define HOST "localhost"

using namespace std;
//code refrence: beej
void * get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int getTcpInfo(){
	 int socket_fd;
    int new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_storage client_addr; // connector's address information
    socklen_t sin_size;
    int flag=1;
    int res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
 // hints.ai_flags = AI_PASSIVE; // use my IP

    res = getaddrinfo(HOST,CLIENTPORT, &hints, &server_info);
    if (res!= 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int))< 0) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(socket_fd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");

    }
    freeaddrinfo(server_info); // all done with this structure

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    //std::cout<<"The AWS is up and running."<<endl;

 
        //receive all the inforamtion from client
        sin_size = sizeof client_addr;   
        new_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");           
            exit(1);
        }
		int p_num;
		double tran_delay;
        recv(new_fd, &p_num, sizeof(p_num), 0);
		recv(new_fd, &tran_delay, sizeof(tran_delay), 0);
		cout<<"The client has received results from AWS: "<<endl;
			std::cout << setfill('-') << setw(75) << '-' << endl;
    std::cout.fill(' ');         //print out the result and then close the socket!
    std::cout << left << setw(15) << "Destination" << setw(15)  << "Min Length" << setw(15) <<"Tt"<<setw(15)<<"Tp"<<setw(15)<<"Delay"<<setw(15)<< endl;
    std::cout << setfill('-') << setw(75) << '-' << endl;
    std::cout<<setfill(' ');
		for(int i=0;i<p_num;++i){
			int des;
			double dis;
			double prop_delay;
			double delay;
			recv(new_fd, &des, sizeof(des), 0);
			recv(new_fd, &dis, sizeof(dis), 0);
			recv(new_fd, &prop_delay, sizeof(prop_delay), 0);
			recv(new_fd, &delay, sizeof(delay), 0);
          std::cout << left << setw(15) << des << setw(15) << dis << setw(15) <<setprecision(2)<<tran_delay<<setw(15)<<setprecision(2)<<prop_delay<<setw(15)<<setprecision(2)<<delay<<setw(15)<< endl;
		
		}
		std::cout << setfill('-') << setw(75) << '-' << endl;
        std::cout<<setfill(' ');
		return 0;
}
//code refrence: beej
int main(int argc, char* argv[]){	

	int socket_fd = 0;
	char Map_ID = *(argv[1]);
    int Source_Vertex_Index = atoi(argv[2]);
    double File_Size = atof(argv[3]);

	struct addrinfo hints, *server_info, *p;

	memset(&hints, 0, sizeof (hints));
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

	printf("The client has sent query to AWS using TCP over port %s: start vertex %d; map %c; file size %lf.\n", AWSPORT, Source_Vertex_Index, Map_ID, File_Size);

    close(socket_fd);
    getTcpInfo();
    return 0;
}
