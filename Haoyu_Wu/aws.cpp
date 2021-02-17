
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <vector>
#include <iomanip>

#include "def.h"

using namespace std;

int Source_Vertex_Index;
double File_Size;
// vector<double> Tt;
// vector<double> Tp;
char Map_ID;
double tran_speed;
double prop_speed;
vector<pair<int,double> > vector_distance;
double trans_delay;
vector< pair<double,double> >vector_prop_delay;




void getUDPconnectionwithserverA(){
    int WHYsocket;
    struct addrinfo hints, *server_info, *p;
    int res;
    int numbytes;
    int flag;
    int point_count;
    //vector<pair<double,int>> minDistance;
    vector<int> Destination;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((res = getaddrinfo(HOST,PORTA,&hints,&server_info))!=0){
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(res));
        return ;
    }
    for (p = server_info;p!=NULL;p = p->ai_next){
        if ((WHYsocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("talker socket;");
            continue;
        }
        break;
    }
    if (p==NULL){
        fprintf(stderr, "talker:failed to bind socket\n" );
        return ;
    }
    numbytes = sendto(WHYsocket, &Map_ID,sizeof(Map_ID),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent map id to server A");
    }
    numbytes = sendto(WHYsocket, &Source_Vertex_Index,sizeof(Source_Vertex_Index),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent starting vertex to server A");
    }

   std::cout << "The AWS has sent map ID and starting vertex to server A using UDP over port "<<PORTA << endl;

    numbytes = recvfrom(WHYsocket, &point_count, sizeof(point_count),0,p->ai_addr,&(p->ai_addrlen));
    if(numbytes == -1){
        perror("The AWS has not received Destination from server A");
        exit(1);
    }
     numbytes = recvfrom(WHYsocket, &prop_speed, sizeof(prop_speed),0,p->ai_addr,&(p->ai_addrlen));
     numbytes = recvfrom(WHYsocket, &tran_speed, sizeof(tran_speed),0,p->ai_addr,&(p->ai_addrlen));

          int des;
        double dis;
    for(int i=0;i<point_count;++i){
           numbytes = recvfrom(WHYsocket, &des, sizeof(des),0,p->ai_addr,&(p->ai_addrlen));
           numbytes = recvfrom(WHYsocket, &dis, sizeof(dis),0,p->ai_addr,&(p->ai_addrlen));
           vector_distance.emplace_back (des,dis);
    }
    // numbytes = recvfrom(WHYsocket, &minDistance, sizeof(minDistance),0,NULL,NULL);
    // if(numbytes == -1){
    //     perror("The AWS has received shortest path from server A");
    //     exit(1);
    // }
   

    std::cout << "The AWS has received shortest path from server A" << endl;

    std::cout << setfill('-') << setw(50) << '-' << endl;
    std::cout.fill(' ');         //print out the result and then close the socket!
    std::cout << left << setw(15) << "Destination" << setw(15)  << "Min Length" << setw(15) << endl;
    std::cout << setfill('-') << setw(50) << '-' << endl;
    std::cout<<setfill(' ');
    for (int i = 0;i < point_count;i++) {
       std:: cout << left << setw(15) << vector_distance[i].first << setw(15) << vector_distance[i].second<<endl;
        // cout << left << fixed << setw(15) << setprecision(2) << Tt[i];
    }
    std::cout << setfill(' ')<<setw(50)<<'-'<<endl;
    std::cout.fill(' ');
    close(WHYsocket);
    return ;

}



int  getUDPconnectionwithserverB(){
    int WHYsocket;
    struct addrinfo hints, *server_info, *p;
    int res;
    int numbytes;
    int flag;
    int point_count;
    int Source_Vertex_Index;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((res = getaddrinfo(HOST,PORTB,&hints,&server_info))!=0){
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(res));
        return 0;
    }
    for (p = server_info;p!=NULL;p = p->ai_next){
        if ((WHYsocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("talker socket;");
            continue;
        }
        break;
    }
    if (p==NULL){
        fprintf(stderr, "talker:failed to bind socket\n" );
        return 0;
    }
             int point_num=vector_distance.size();
           numbytes = sendto(WHYsocket, &point_num,sizeof(point_num),0,p->ai_addr,p->ai_addrlen);

            numbytes=sendto(WHYsocket,&prop_speed,sizeof(prop_speed),0,p->ai_addr,p->ai_addrlen);
            numbytes=sendto(WHYsocket,&tran_speed,sizeof(tran_speed),0,p->ai_addr,p->ai_addrlen);
            numbytes=sendto(WHYsocket,&File_Size,sizeof(File_Size),0,p->ai_addr,p->ai_addrlen);
        for(int i=0;i<point_num;i++){
            int des=vector_distance[i].first;
            double dis=vector_distance[i].second;
            numbytes = sendto(WHYsocket, &des,sizeof(des),0,p->ai_addr,p->ai_addrlen);
            numbytes=sendto(WHYsocket,&dis,sizeof(dis),0,p->ai_addr,p->ai_addrlen);
        }

    std::cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port  "<<PORTB << endl;
  
   
    numbytes = recvfrom(WHYsocket, &point_count, sizeof(point_count),0,p->ai_addr,&(p->ai_addrlen));
    numbytes = recvfrom(WHYsocket, &trans_delay, sizeof(trans_delay),0,p->ai_addr,&(p->ai_addrlen));
    double prop_delay;
    double delay;
    for(int i=0;i<point_count;++i){      
	numbytes = recvfrom(WHYsocket, &(prop_delay),sizeof(prop_delay),0,p->ai_addr,&(p->ai_addrlen));
	numbytes = recvfrom(WHYsocket, &(delay),sizeof(delay),0,p->ai_addr,&(p->ai_addrlen));
    vector_prop_delay.emplace_back(prop_delay,delay);
	 }
    std::cout << "The AWS has received delay from server B" << endl;

       std:: cout << setfill('-') << setw(75) << '-' << endl;
        std::cout.fill(' ');         //print out the result and then close the socket!
       std:: cout << left << setw(15) << "Destination" << setw(15)  << "Tt" << setw(15) << "Tp" << setw(15) << "Delay" << endl;
        std::cout << setfill('-') << setw(75) << '-' << endl;
       std:: cout.fill(' ');
    for (int i = 0;i < point_count;i++) {
        cout << left << setw(15) << vector_distance[i].first << setw(15) ;
        // cout << left << fixed << setw(15) << setprecision(2) << trans_delay;
        // cout << left << fixed << setw(15) << setprecision(2) << vector_prop_delay[i].first;
        // cout << left << fixed << setw(15) << setprecision(2) << vector_prop_delay[i].second << endl;
        cout << left << fixed << setw(15)<<setprecision(2) << trans_delay;
        cout << left << fixed << setw(15)<<setprecision(2) << vector_prop_delay[i].first;
        cout << left << fixed << setw(15)<<setprecision(2) << vector_prop_delay[i].second << endl;
    }
    cout << setfill(' ')<<setw(75)<<'-'<<endl;
    cout.fill(' ');
    cout<<"#"<<endl<<endl;
    close(WHYsocket);
    return point_count;

}

void * get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int sendTcpInfo(int p_num){
    int socket_fd = 0;

	struct addrinfo hints, *server_info, *p;

	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int res = getaddrinfo(HOST, CLIENTPORT, &hints, &server_info);

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
	freeaddrinfo(server_info); // all done with this structure
	

	send(socket_fd, &p_num, sizeof(p_num), 0);
    send(socket_fd, &trans_delay, sizeof(trans_delay), 0);
    for(int i=0;i<p_num;i++){
    send(socket_fd, &(vector_distance[i].first), sizeof(vector_distance[i].first), 0);
    send(socket_fd, &(vector_distance[i].second), sizeof(vector_distance[i].second), 0);
    send(socket_fd, &(vector_prop_delay[i].first), sizeof(vector_prop_delay[i].first), 0);
    send(socket_fd, &(vector_prop_delay[i].second), sizeof(vector_prop_delay[i].second), 0);
    }
	// send(socket_fd, &Source_Vertex_Index, sizeof(Source_Vertex_Index), 0);
	// send(socket_fd, &File_Size, sizeof(File_Size), 0);

	cout<<"The AWS has sent calculated delay to client using TCP over port "<<CLIENTPORT<<endl;
    close(socket_fd);
    return 0;
}

int main(){
    //from Beej
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

    res = getaddrinfo(HOST, AWSPORT, &hints, &server_info);
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

    cout<<"The AWS is up and running."<<endl;

    while(1){
        Map_ID=' ';
        Source_Vertex_Index=-1;
        File_Size=-1;
        vector<pair<int,double>  >().swap(vector_distance);
        tran_speed=0;
        prop_speed=0;
        trans_delay=0;
        //receive all the inforamtion from client
        sin_size = sizeof client_addr;   
        new_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");           
            exit(1);
        }
        recv(new_fd, &Map_ID, sizeof(Map_ID), 0);
        recv(new_fd, &Source_Vertex_Index, sizeof(Source_Vertex_Index), 0);
        recv(new_fd, &File_Size, sizeof(File_Size), 0);
        cout<<"The AWS has received map ID "<<Map_ID<<", start vertex "<<Source_Vertex_Index<<", and file size "<<File_Size<<", from the client using TCP over port "<<CLIENTPORT<<endl;
        close(new_fd); 
        getUDPconnectionwithserverA();
       int p_num= getUDPconnectionwithserverB();
        sendTcpInfo(p_num);
    }
    /*
    getUDPconnectionwithserverA();
    getUDPconnectionwithserverB();
    */


    return 0;
}

