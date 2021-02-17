#include <stdio.h>
#include <stdlib.h>
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

using namespace std;

#define CLIENTPORT "24595"   //TCP port
#define UDPPORT "23595"     //UDP port
#define HOST "localhost"
#define BACKLOG 10 // how many pending connections queue will hold
#define PORTA "21595"
#define PORTB "22595"


int Source_Vertex_Index, File_Size;
char Map_ID;

//beej
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

//beej
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getUDPconnectionwithserverA(){
    int WHYsocket;
    struct addrinfo hints, *server_info, *p;
    int res;
    int numbytes;
    int flag;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((res = getaddrinfo(HOST,PORTA,&hints,&server_info))!=0){
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(res));
        return 1;
    }
    for (p = server_info;p!=NULL;p = p->ai_next){
        if ((WHYsocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("talker socket;")
            continue;
        }
        break;
    }
    if (p==NULL){
        fprintf(stderr, "talker:failed to bind socket\n" );
        return 1;
    }
    int numbytes = sendto(WHYsocket, &Map_ID,sizeof(Map_ID),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent map id to server A");
    }
    int numbytes = sendto(WHYsocket, &Source_Vertex_Index,sizeof(Source_Vertex_Index),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent starting vertex to server A");
    }

    cout << "The AWS has sent map ID and starting vertex to server A using UDP over port 23595." << endl;
    //need to be filled in
    int numbytes = recvfrom(WHYsocket, &Tt, sizeof(Tt),0,NULL,NULL);
    if(numbytes == -1){
        perror("The AWS has received Destination from server A");
        exit(1);
    }
    int numbytes = recvfrom(WHYsocket, &Tp, sizeof(Tp),0,NULL,NULL);
    if(numbytes == -1){
        perror("The AWS has received shortest path from server A");
        exit(1);
    }
   

    cout << "The AWS has shortest path from server A;" << endl;

    cout << setfill('-') << setw(50) << '-' << endl;
    cout.fill(' ');         //print out the result and then close the socket!
    cout << left << setw(15) << "Destination" << setw(15)  << "Min Length" << setw(15) << endl;
    cout << setfill('-') << setw(50) << '-' << endl;
    cout.fill(' ');
    for (int i = 0;i < numVertices;i++) {
        cout << left << setw(15) << vertexName[i] << setw(15) << minDis[i];
        cout << left << fixed << setw(15) << setprecision(2) << Tt;
    }
    cout << setfill('')<<setw(50)<<'-'<<endl;
    cout.fill('');
    close(WHYsocket);
    return 0;

}
void getUDPconnectionwithserverB(){
    int WHYsocket;
    struct addrinfo hints, *server_info, *p;
    int res;
    int numbytes;
    int flag;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((res = getaddrinfo(HOST,PORTB,&hints,&server_info))!=0){
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(res));
        return 1;
    }
    for (p = server_info;p!=NULL;p = p->ai_next){
        if ((WHYsocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("talker socket;")
            continue;
        }
        break;
    }
    if (p==NULL){
        fprintf(stderr, "talker:failed to bind socket\n" );
        return 1;
    }
    int numbytes = sendto(WHYsocket, &prop_speed,sizeof(prop_speed),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent the propagation speed to server B");
    }
    int numbytes = sendto(WHYsocket, &tran_speed,sizeof(tran_speed),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent the transmission speed to server B");
    }
    int numbytes = sendto(WHYsocket, &File_Size,sizeof(File_Size),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent the filesize to server B");
    }
    int numbytes = sendto(WHYsocket, &Source_Vertex_Index,sizeof(Source_Vertex_Index),0,p->ai_addr,p->ai_addrlen);
    if (numbytes == -1){
        perror("The AWS has sent the filesize to server B");
    }

    int tmp = 0;
    double tmp_value;
    for (int i =0;i<Source_Vertex_Index;i++){
        tmp = Source_Vertex_Index[i];
        int numbytes = sendto(WHYsocket,&tmp,sizeof(tmp),0,p->ai_addr,p->ai_addrlen);
        if (numbytes ==-1){
            perror("AWS has sent vertex to server B");
            exit(1);
        }
    }
    for (int i =0; i<Source_Vertex_Index;i++){
        tmp = shortestPath[i];
        int numbytes = sendto(WHYsocket,&tmp,sizeof(tmp),0,p->ai_addr,p->ai_addrlen);
        if (numbytes ==-1){
            perror("AWS has sent shortestPath to server B");
            exit(1);
        }
    }
    cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port 23595." << endl;
    int numbytes = recvfrom(WHYsocket, &Tt, sizeof(Tt),0,NULL,NULL);
    if(numbytes == -1){
        perror("The AWS has received Tt from server B");
        exit(1);
    }
    int numbytes = recvfrom(WHYsocket, &Tp, sizeof(Tp),0,NULL,NULL);
    if(numbytes == -1){
        perror("The AWS has received Tp from server B");
        exit(1);
    }
    int numbytes = recvfrom(WHYsocket, &delay, sizeof(delay),0,NULL,NULL);
    if(numbytes == -1){
        perror("The AWS has received delay from server B");
        exit(1);
    }

    cout << "The AWS has received delay from server B;" << endl;

    cout << setfill('-') << setw(50) << '-' << endl;
    cout.fill(' ');         //print out the result and then close the socket!
    cout << left << setw(15) << "Destination" << setw(15)  << "Tt" << setw(15) << "Tp" << setw(15) << "Delay" << endl;
    cout << setfill('-') << setw(50) << '-' << endl;
    cout.fill(' ');
    for (int i = 0;i < numVertices;i++) {
        cout << left << setw(15) << vertexName[i] << setw(15) << shortestPath[i];
        cout << left << fixed << setw(15) << setprecision(2) << Tt[i];
        cout << left << fixed << setw(15) << setprecision(2) << Tp[i];
        cout << left << fixed << setw(15) << setprecision(2) << Tp[i] + Tt << endl;
    }
    cout << setfill('')<<setw(50)<<'-'<<endl;
    cout.fill('');
    close(WHYsocket);
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

    res = getaddrinfo(HOST, CLIENTPORT, &hints, &server_info);
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
    printf( "The AWS is up and running.\n");

    while(1){
        //receive all the inforamtion from client
        sin_size = sizeof client_addr;   
        new_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");           
            exit(1);
        }
        printf("Accept Successfully!\n");
        recv(new_fd, &Map_ID, sizeof(Map_ID), 0);
        recv(new_fd, &Source_Vertex_Index, sizeof(Source_Vertex_Index), 0);
        recv(new_fd, &File_Size, sizeof(File_Size), 0);
        printf("The AWS has received Map_ID %c, start vertex %d, and file size %d, from the client using TCP over port %s.\n", Map_ID, Source_Vertex_Index, File_Size, CLIENTPORT);
        close(new_fd); 
    }
    getUDPconnectionwithserverA();
    getUDPconnectionwithserverB();

    return 0;
}

