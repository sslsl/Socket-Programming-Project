
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <iomanip>
#include <math.h>

//#include "aws.h"
using namespace std;


#define LOCALIP "127.0.0.1" // IP Address of Host
#define TCPPORT 24984 // TCP Port # client connects to
#define UDPPORT 23984 // UDP Port # backend servers connects to
#define SERVERAPORT 21984
#define SERVERBPORT 22984
#define BACKLOG 3 // backlog of pending connections for listen
#define BUFLEN 1000

char buf [BUFLEN];
char mapID [BUFLEN];
char vertexIndex[BUFLEN];
char fileSize[BUFLEN];
int recvLen1, sendLen;
vector< pair <int, int> > shortestPathPairs; // (node, distance (km)) in ascending distance order
double propSpeed; // in km/s
double transSpeed; // in Bytes/s
vector<double> propDelay;
double transDelay;
vector<double> totDelay;

struct sockaddr_in awsAddrTCP, awsAddrUDP, clientAddr, serverAAddr, serverBAddr;
int aws_TCP_sockfd, aws_UDP_sockfd, new_aws_TCP_sockfd;


// Initializes TCP Port on AWS
void init_TCP(){
    
    // *** 1. CREATE SOCKET ***
       if ( (aws_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
           perror("Error creating TCP socket");
           exit(EXIT_FAILURE);
       }
       // specify AWS address
       awsAddrTCP.sin_family = AF_INET;
       //AWS Port #
       awsAddrTCP.sin_port = htons(TCPPORT);
       //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
       awsAddrTCP.sin_addr.s_addr = inet_addr(LOCALIP);
       
       // *** 2. BIND SOCKET ***
       
       if (::bind(aws_TCP_sockfd, (struct sockaddr *) &awsAddrTCP, sizeof(awsAddrTCP)) == -1 ){
           close(aws_TCP_sockfd);
           perror("Error binding TCP socket");
           exit(EXIT_FAILURE);
       }
    // *** 3. LISTEN FOR CONNECTIONS ***
    
    if (listen(aws_TCP_sockfd, BACKLOG) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void init_UDP(){
    // *** 1. CREATE SOCKET ***
    if ( (aws_UDP_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Error creating UDP socket");
        exit(EXIT_FAILURE);
    }

    // specify AWS address
    
    awsAddrUDP.sin_family = AF_INET;
    //AWS Port #
    awsAddrUDP.sin_port = htons(UDPPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    awsAddrUDP.sin_addr.s_addr = inet_addr(LOCALIP);
    
    // *** 2. BIND SOCKET ***
    
    if (::bind(aws_UDP_sockfd, (struct sockaddr *) &awsAddrUDP, sizeof(awsAddrUDP)) == -1 ){
        close(aws_UDP_sockfd);
        perror("Error binding UDP socket");
        exit(EXIT_FAILURE);
    }
}

void acceptFromClient(){
    //clientlen from CMU class notes
    socklen_t clientLen = sizeof(clientAddr);
    //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    if ( (new_aws_TCP_sockfd = accept(aws_TCP_sockfd,(struct sockaddr *) &clientAddr, &clientLen)) == -1){
        perror("Error accepting socket");
        exit(EXIT_FAILURE);
    }
}

void recvFromClient(){
    
    recvLen1 = recv(new_aws_TCP_sockfd, mapID, BUFLEN, 0);
    if (recvLen1 == -1){
        perror("Error receiving message from client");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    mapID[recvLen1] = '\0';
//    cout << "Debug M: " << mapID << endl;

    recvLen1 = recv(new_aws_TCP_sockfd, vertexIndex, BUFLEN, 0);
    if (recvLen1 == -1){
        perror("Error receiving message from client");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    vertexIndex[recvLen1] = '\0';
//    cout << "Debug V: " << vertexIndex << endl;
    recvLen1 = recv(new_aws_TCP_sockfd, fileSize, BUFLEN, 0);
    if (recvLen1 == -1){
        perror("Error receiving message from client");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    fileSize[recvLen1] = '\0';
//    cout << "Debug F: " << fileSize << endl;
    
    
//    if ((recvLen1 = recv(client_sockfd, buf, BUFLEN, 0)) < 0){
//        perror("Error receiving message from client");
//        exit(EXIT_FAILURE);
//    }
//
//    buf[recvLen1] = '\0';
//    cout << buf << endl;
}
 
// Sets port and IP of serverA and serverB
void setServerAB(){
//    Server A
    serverAAddr.sin_family = AF_INET;
    //AWS Port #
    serverAAddr.sin_port = htons(SERVERAPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    serverAAddr.sin_addr.s_addr = inet_addr(LOCALIP);
    
//    Server B
    serverBAddr.sin_family = AF_INET;
    //AWS Port #
    serverBAddr.sin_port = htons(SERVERBPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    serverBAddr.sin_addr.s_addr = inet_addr(LOCALIP);
}

// Sends Map ID and Vertex index to Server A via UDP
void sendToA(){
    
    if ((sendLen = sendto(aws_UDP_sockfd, mapID, strlen(mapID), 0, (struct sockaddr *) &serverAAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server A from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    if ((sendLen = sendto(aws_UDP_sockfd, vertexIndex, strlen(vertexIndex), 0, (struct sockaddr *) &serverAAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server A from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    cout << "The AWS has sent map ID and starting vertex to server A using UDP over port " << SERVERAPORT << endl;
}

void recvFromA(){
    char destBuf[BUFLEN];
    char lenBuf[BUFLEN];
    socklen_t serverALen = sizeof(serverAAddr);
    
    // recv prop speed
    
    if ((recvLen1 = recvfrom(aws_UDP_sockfd, buf, BUFLEN, 0, (struct sockaddr *) &serverAAddr, &serverALen )) < 0){
        perror("Error receiving message from Server A");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    buf[recvLen1] = '\0';
    
    // if buf == -2 then the map or source node is incorrect and AWS should abort
    if (atoi(buf) == -2){
        cerr << "Received error from Server A: Map or source node invalid\n";
        // send error to Server B to terminate process
        int sendLen;
        if ((sendLen = sendto(aws_UDP_sockfd, "-2", strlen("-2"), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to Server B from AWS");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        // send error to client to terminate process
        if (send(new_aws_TCP_sockfd, "-2", strlen("-2"), 0) == -1){
            perror("Error sending data to client");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_FAILURE);
    }
    
    propSpeed = atof(buf);
//    cout << "DEBUG: " << propSpeed << endl;
    
    //  recv trans speed
    if ((recvLen1 = recvfrom(aws_UDP_sockfd, buf, BUFLEN, 0, (struct sockaddr *) &serverAAddr, &serverALen )) < 0){
        perror("Error receiving message from Server A");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
       }
    buf[recvLen1] = '\0';
    transSpeed = atof(buf);
//    cout << "DEBUG: " << transSpeed << endl;
    
    memset(buf, '0', sizeof(buf));
    int recvDone = 0; // 0 = not finished receiving, 1 = finished receiving
    while (!recvDone){
        
        if ((recvLen1 = recvfrom(aws_UDP_sockfd, destBuf, BUFLEN, 0, (struct sockaddr *) &serverAAddr, &serverALen )) < 0){
            perror("Error receiving message from Server A");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        destBuf[recvLen1] = '\0';
//        cout << "DEBUG dest: " << destBuf << endl;
        
        // receive min length if destination received is valid
        if (destBuf[0] != '\0'){
            if ((recvLen1 = recvfrom(aws_UDP_sockfd, lenBuf, BUFLEN, 0, (struct sockaddr *)&serverAAddr, &serverALen )) < 0){
                perror("Error receiving message from Server A");
                close(new_aws_TCP_sockfd);
                exit(EXIT_FAILURE);
            }
            lenBuf[recvLen1] = '\0';
//            cout << "DEBUG len: " << lenBuf << endl;

        
            shortestPathPairs.push_back(make_pair(atoi(destBuf), atoi(lenBuf)) );
        }
        else{
            //toggle recvDone
            recvDone = 1;
        }
         
//        cout << "DEBUG: " << recvDone << endl;
    } // end while
    
    

    cout << "The AWS has received shortest path from server A: " << endl << "--------------------------------" << endl << left << setw(20) << "Destination" << "Min Length" << endl << "--------------------------------" << endl;
    



    for (auto it = shortestPathPairs.begin(); it != shortestPathPairs.end(); it++){
        cout << setw(20) << it->first << it->second << endl;
    }
    cout << "--------------------------------" << endl;
}

// Sends propspeed, transspeed, and Dijkstra result to Server B via UDP
void sendToB(){
    char destBuf[BUFLEN];
    char lenBuf[BUFLEN];
   
    // send file size
    
    if ((sendLen = sendto(aws_UDP_sockfd, fileSize, strlen(fileSize), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server B from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
      }
    
    memset(fileSize, '\0' , sizeof(fileSize));
    
    // send prop speed
    sprintf(buf, "%f", propSpeed);
    if ((sendLen = sendto(aws_UDP_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server B from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    // send trans speed
     sprintf(buf, "%f", transSpeed);
    if ((sendLen = sendto(aws_UDP_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server B from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    // send shortest paths
    
    for(auto it = shortestPathPairs.begin(); it != shortestPathPairs.end(); it++){
        // send dest
        sprintf(destBuf, "%d", it->first);
        if ((sendLen = sendto(aws_UDP_sockfd, destBuf, strlen(destBuf), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to Server B from AWS");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        memset(destBuf, '\0', sizeof(lenBuf));
        
        // send length
        sprintf(lenBuf, "%d", it->second);
        if ((sendLen = sendto(aws_UDP_sockfd, lenBuf, strlen(lenBuf), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to Server B from AWS");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
     
        memset(lenBuf, '\0', sizeof(lenBuf));
    }
    memset(buf, '\0', sizeof(buf));
    // Send NULL char to signify end of communication
    if ( ( sendLen = sendto(aws_UDP_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serverBAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to Server B from AWS");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    
    cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port " << SERVERBPORT <<"." << endl;
}

void recvFromB(){
    char propBuf[BUFLEN];
    char transBuf[BUFLEN];
    char totBuf[BUFLEN];
    socklen_t serverBLen = sizeof(serverBAddr);
    
    memset(buf, '0', sizeof(buf));
    
    //receive transmission delay once
    if ((recvLen1 = recvfrom(aws_UDP_sockfd, transBuf, BUFLEN, 0, (struct sockaddr *)&serverBAddr, &serverBLen )) < 0){
        perror("Error receiving message from Server B");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    transBuf[recvLen1] = '\0';
    
    // Exception handling
    // if buf == -1 then too large
    if (atoi(transBuf) == -1){
        cerr << "Received error from Server B: File size too large\n";
        
        // send error to client to terminate process
        if (send(new_aws_TCP_sockfd, "-1", strlen("-1"), 0) == -1){
            perror("Error sending data to client");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_FAILURE);
    }
    
    transDelay = atof(transBuf);
    
    int recvDone = 0; // 0 = not finished receiving, 1 = finished receiving
    while (!recvDone){
        
        // receive propagation delay
        if ((recvLen1 = recvfrom(aws_UDP_sockfd, propBuf, BUFLEN, 0, (struct sockaddr *) &serverBAddr, &serverBLen )) < 0){
            perror("Error receiving message from Server B");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        propBuf[recvLen1] = '\0';
//        cout << "DEBUG dest: " << destBuf << endl;
        
        // receive min length if destination received is valid
        if (propBuf[0] != '\0'){
            
            // receive total delay
            if ((recvLen1 = recvfrom(aws_UDP_sockfd, totBuf, BUFLEN, 0, (struct sockaddr *)&serverBAddr, &serverBLen )) < 0){
                perror("Error receiving message from Server B");
                close(new_aws_TCP_sockfd);
                exit(EXIT_FAILURE);
            }
            totBuf[recvLen1] = '\0';
            
            propDelay.push_back(atof(propBuf));
            totDelay.push_back(atof(totBuf));
           
//            cout << "DEBUG tot: " << totBuf << endl;
      }
        else{
            //toggle recvDone
            recvDone = 1;
        }
    } // end while

    int sizet = to_string(transDelay).length();
    int sizep = to_string(propDelay[propDelay.size()-1]).length();
    int sizetot = to_string(totDelay[totDelay.size()-1]).length();
    
    cout << "The AWS has received delays from server B:" << endl;
//    cout << "--------------------------------------------" << endl << "Destination" << setw(sizet) << "Tt" << setw(sizep) << "Tp" << setw(sizetot) << "Delay (sec)" << endl << "--------------------------------------------" << endl;
    cout << "-----------------------------------------------------------------------------" << endl << left << setw(20) << "Destination" << setw(20) << "Tt" << setw(20) << "Tp" << setw(20) << "Delay" << endl << "-----------------------------------------------------------------------------" << endl;
    cout << fixed;
    
    
    for(int i = 0; i < shortestPathPairs.size(); i++){
//        cout << shortestPathPairs[i].first << setw(30) << setprecision(2) << transDelay << setw(sizep) << propDelay[i] << setw(sizetot) << totDelay[i] << endl;
        cout << left << setw(20) << shortestPathPairs[i].first << setw(20) << setprecision(2) << transDelay << setw(20) << propDelay[i] << setw(20) <<  totDelay[i] << endl;
    }

    cout << "-----------------------------------------------------------------------------" << endl;
    
}

void sendToClient(){
    char transBuf [BUFLEN];
    char destBuf [BUFLEN];
    char lenBuf[BUFLEN];
    char propBuf [BUFLEN];
    char totBuf [BUFLEN];
    
    
    // send transmission delay
    sprintf(transBuf,"%f", transDelay);
    if (send(new_aws_TCP_sockfd, transBuf, strlen(transBuf), 0) == -1){
        perror("Error sending data to client");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < shortestPathPairs.size(); i++){
        
        // send destination node
        for (int i = 0; i < 10000000; i++){}
        sprintf(destBuf,"%d", shortestPathPairs[i].first);
           if (send(new_aws_TCP_sockfd, destBuf, strlen(destBuf), 0) == -1){
               perror("Error sending data to client");
               close(new_aws_TCP_sockfd);
               exit(EXIT_FAILURE);
           }
        // send destination length
        for (int i = 0; i < 10000000; i++){}
        sprintf(lenBuf,"%d", shortestPathPairs[i].second);
           if (send(new_aws_TCP_sockfd, lenBuf, strlen(lenBuf), 0) == -1){
               perror("Error sending data to client");
               close(new_aws_TCP_sockfd);
               exit(EXIT_FAILURE);
           }
        // send propagation delay
        for (int i = 0; i < 10000000; i++){}
        sprintf(propBuf,"%f", propDelay[i]);
        if (send(new_aws_TCP_sockfd, propBuf, strlen(propBuf), 0) == -1){
            perror("Error sending data to client");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        // send total delay
        for (int i = 0; i < 10000000; i++){}
        sprintf(totBuf,"%f", totDelay[i]);
        if (send(new_aws_TCP_sockfd, totBuf, strlen(totBuf), 0) == -1){
            perror("Error sending data to client");
            close(new_aws_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
    }
    
    memset(buf, '\0', sizeof(buf));
    for (int i = 0; i < 10000000; i++){}
    // Send NULL char to signify end of communication
    if (send(new_aws_TCP_sockfd, buf, strlen(buf), 0) == -1){
        perror("Error sending data to client");
        close(new_aws_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    cout << "The AWS has sent calculated delay to client using TCP over port " << TCPPORT << "." << endl;
}

int main (){
    // initialize new port
    init_TCP();
    // Boot up message
    cout << "The AWS is up and running." << endl;
    
    init_UDP();
    
    setServerAB();
    // ********* FOR CLIENT SOCKET CONNECTION *********
    
//      while(1) loop for main process for AWS to send/recv to/from client, serverA, and serverB
    while (1) {
        // *** 4. ACCEPT CONNECTIONS ***
        acceptFromClient();
        // *** 5. RECEIVE DATA FROM CLIENT ***
        recvFromClient();
        //separateClientBuffer();
        
        // output message for receiving data from client
        cout << "The AWS has received map ID " << mapID << ", start vertex " << vertexIndex << " and file size " << fileSize << " from the client using TCP over port " << TCPPORT << endl;
        
        //send map ID / vertex index to serverA
        sendToA();
        // receive shortest path data
        recvFromA();
        //cout << "end" << endl;
        
        // send prop speed, trans speed, file size, and shortest paths to serverB
        sendToB();
        // receive delay from server B
        recvFromB();
        
        // send final result to client
        sendToClient();
               
        //empty shortest path vector
        shortestPathPairs.clear();
        propDelay.clear();
        totDelay.clear();
        transDelay = 0;
        // close client child socket
        close(new_aws_TCP_sockfd);
    } // end of while(1)  
    return EXIT_SUCCESS;
}
