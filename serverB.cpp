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
#include <iomanip>
#include <vector>

//#include "serverB.h"
using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define UDPPORT 23984 // UDP Port # backend servers connects to
#define SERVERBPORT 22984
#define BUFLEN 1000 // Length of socket stream buffer

char buf [BUFLEN];
char mapID [BUFLEN];
char vertexIndex[BUFLEN];
char fileSizeBuf[BUFLEN]; // in bits
int recvLen1;
int sendLen;
char pSpeedBuf[BUFLEN]; // propagation speed
char tSpeedBuf[BUFLEN]; // transmission speed
double propSpeed; // in km/s
double transSpeed; // in Bytes/s
long long fileSize;
vector<double> propDelay;
double transDelay;
vector<double> totDelay;

vector< pair <int, int> > shortestPathPairs; // (node, distance (km)) in ascending distance order
vector< pair <int, double> > totalDelayPairs; // (node, delay (sec)) in ascending delay order

struct sockaddr_in awsAddrUDP;

struct sockaddr_in serverBAddr;
int serverB_sockfd;


void init_UDP(){
    // *** 1. CREATE SOCKET ***
    if ( (serverB_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Error creating UDP socket");
        exit(EXIT_FAILURE);
    }
    
    // specify serverB address
    
    serverBAddr.sin_family = AF_INET;
    //AWS Port #
    serverBAddr.sin_port = htons(SERVERBPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    serverBAddr.sin_addr.s_addr = inet_addr(LOCALIP);
    
    // *** 2. BIND SOCKET ***
    
    if (::bind(serverB_sockfd, (struct sockaddr *) &serverBAddr, sizeof(serverBAddr)) == -1 ){
        close(serverB_sockfd);
        perror("Error binding UDP socket");
        exit(EXIT_FAILURE);
    }
    
    cout << "The Server B is up and running using UDP on port " << SERVERBPORT << "." << endl;
}


void recvFromAWS(){
    char destBuf[BUFLEN];
    char lenBuf[BUFLEN];
    socklen_t awsLen = sizeof(awsAddrUDP);
    memset(buf, '0', sizeof(buf));
    memset(fileSizeBuf, '\0' , sizeof(fileSizeBuf));
    int recvDone = 0; // 0 = not finished receiving, 1 = finished receiving
    
    // Receive file size
    if ((recvLen1 = recvfrom(serverB_sockfd, fileSizeBuf, BUFLEN, 0, (struct sockaddr *) &awsAddrUDP, &awsLen )) < 0){
        perror("Error receiving message from aws");
        exit(EXIT_FAILURE);
        
    }
    fileSizeBuf[recvLen1] = '\0';
    
    // Exception handling
    // if buf == -2 then the map or source node is incorrect and server B should abort
    if (atoi(fileSizeBuf) == -2){
        cerr << "Received error from AWS: Map or source node invalid\n";
        exit(EXIT_FAILURE);
    }
    // Overflow when filesize > 9223372036854775805
    // if buf = -1 then filesize is too big
    if (atoi(fileSizeBuf) == -1){
        perror("Received error from AWS");
        int sendLen;
        // return error to AWS
        if ((sendLen = sendto(serverB_sockfd, "-1", strlen("-1"), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server B");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_FAILURE);
    }
    
    fileSize = atoll(fileSizeBuf);
    
    // Recv 1st propagation speed 2nd transmission speed
    memset(pSpeedBuf, '\0' , sizeof(pSpeedBuf));
    if ((recvLen1 = recvfrom(serverB_sockfd, pSpeedBuf, BUFLEN, 0, (struct sockaddr *) &awsAddrUDP, &awsLen )) < 0){
        perror("Error receiving message from aws");
        exit(EXIT_FAILURE);
    }
    pSpeedBuf[recvLen1] = '\0';
    propSpeed = atof(pSpeedBuf);
    
    memset(tSpeedBuf, '\0' , sizeof(tSpeedBuf));
    if ((recvLen1 = recvfrom(serverB_sockfd, tSpeedBuf, BUFLEN, 0, (struct sockaddr *)&awsAddrUDP, &awsLen )) < 0){
        perror("Error receiving message from aws");
        exit(EXIT_FAILURE);
    }
    tSpeedBuf[recvLen1] = '\0';
    
    transSpeed = atof(tSpeedBuf);
    
    // receive edge data
    memset(destBuf, '\0' , sizeof(destBuf));
    memset(lenBuf, '\0' , sizeof(lenBuf));
    while (!recvDone){
        
        if ((recvLen1 = recvfrom(serverB_sockfd, destBuf, BUFLEN, 0, (struct sockaddr *) &awsAddrUDP, &awsLen )) < 0){
            perror("Error receiving message from aws");
            exit(EXIT_FAILURE);
        }
        destBuf[recvLen1] = '\0';
//        cout << "dest: " << destBuf << endl;
        
        // receive min length if destination received is valid
        if (destBuf[0] != '\0'){
            if ((recvLen1 = recvfrom(serverB_sockfd, lenBuf, BUFLEN, 0, (struct sockaddr *)&awsAddrUDP, &awsLen )) < 0){
                perror("Error receiving message from aws");
                exit(EXIT_FAILURE);
            }
            lenBuf[recvLen1] = '\0';
//            cout << "len: " << lenBuf << endl;
            
            shortestPathPairs.push_back(make_pair(atoi(destBuf), atoi(lenBuf)) );
        }
        else{
            //toggle recvDone
            recvDone = 1;
        }
        
//        cout << recvDone << endl;
        
    } // end while
    
    
    cout << "The Server B has received data for calculation: "<< endl;
    cout << fixed;
    cout << "* Propagation speed: " << setprecision(2) << propSpeed << " km/s" << endl;;
    cout << "* Transmission speed "<< setprecision(2) << transSpeed << " Bytes/s" << endl;
    
    for (auto it = shortestPathPairs.begin(); it != shortestPathPairs.end(); it++){
        cout << "* Path length for destination " << it->first << ": " << it->second << endl;
    }
   
}


void calcDelay(){
    int i = 0;
    // convert file size in bits to bytes
    // trans delay (same for all nodes) (seconds)
    transDelay = fileSize / (8 * transSpeed);
    for (auto it = shortestPathPairs.begin(); it != shortestPathPairs.end(); it++){
        // prop delay =  (distance / propagation speed) (seconds)
        propDelay.push_back(it->second / propSpeed);
        
        // total delay (transmission delay + propagation delay)
        totDelay.push_back(propDelay[i] + transDelay);
        
        totalDelayPairs.push_back(make_pair(it->first, totDelay[i]));
        i++;
    }
    
    cout << "The Server B has finished the calculation of the delays: " << endl;
    cout << "-------------------------------------" << endl << left << setw(20) << "Destination" << "Delay (sec)" << endl << "-------------------------------------" << endl;
    
    for(auto it = totalDelayPairs.begin(); it != totalDelayPairs.end(); it++){
        cout << left << setw(20) << it->first << setw(20) << it->second << endl;
    }
    cout << "-------------------------------------" << endl;
}

void sendToAWS(){
    
    // send delays
    char propBuf[BUFLEN];
    char transBuf[BUFLEN];
    char totBuf[BUFLEN];
    
    // send transmission delay
    sprintf(transBuf, "%f", transDelay);
    if ((sendLen = sendto(serverB_sockfd, transBuf, strlen(transBuf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to AWS from Server B");
        exit(EXIT_FAILURE);
    }
    
    memset(transBuf, '\0', sizeof(transBuf));
    
    for(int i = 0; i < propDelay.size(); i++){
        // send propagation delay
        sprintf(propBuf, "%f", propDelay[i]);
        if ((sendLen = sendto(serverB_sockfd, propBuf, strlen(propBuf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server B");
            exit(EXIT_FAILURE);
        }
        memset(propBuf, '\0', sizeof(propBuf));
        
        
        // send total delay
        sprintf(totBuf, "%f", totDelay[i]);
        if ((sendLen = sendto(serverB_sockfd, totBuf, strlen(totBuf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server B");
            exit(EXIT_FAILURE);
        }
        memset(totBuf, '\0', sizeof(totBuf));
    }
    memset(buf, '\0', sizeof(buf));
    // Send NULL char to signify end of communication
    if ( ( sendLen = sendto(serverB_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to AWS from Server B");
        exit(EXIT_FAILURE);
    }
    cout << "The Server B has finished sending the output to AWS" << endl;
}


int main(){   
    init_UDP();  
    while(1){
        
        recvFromAWS();
        calcDelay();
        sendToAWS();
        // erase path data
        shortestPathPairs.clear();
        propDelay.clear();
        totDelay.clear();
        totalDelayPairs.clear();
    }
    return 0;
}