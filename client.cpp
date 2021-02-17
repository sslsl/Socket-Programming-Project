
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
#include <ctype.h>
#include <vector>
//#include "client.h"

using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define AWSPORT 24984 // TCP Port # of AWS server
#define BUFLEN 1000 // Length of socket stream buffer


struct sockaddr_in awsAddrTCP;
int aws_TCP_sockfd;

struct sockaddr_in clientAddr;
int client_sockfd;

vector< pair <int, int> > shortestPathPairs; // (node, distance (km)) in ascending distance order
vector<double> propDelay; // seconds
double transDelay; // seconds
vector<double> totDelay; // seconds

void init_TCP(){
    // *** CREATE SOCKET ***  
    aws_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // *** CONNECT SOCKET ***
    
    // specify AWS address
    
    awsAddrTCP.sin_family = AF_INET;
    //AWS Port #
    awsAddrTCP.sin_port = htons(AWSPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    //awsAddrTCP.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;
    awsAddrTCP.sin_addr.s_addr = inet_addr(LOCALIP);
    
    /*
     //DEBUG: check IP
     char *ip = inet_ntoa(awsAddrTCP.sin_addr);
     printf("Debug: Ip Address -> %s\n", ip);
     */
    
    if ( connect(aws_TCP_sockfd, (struct sockaddr *) &awsAddrTCP, sizeof(awsAddrTCP)) == -1){
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }
    
    
    /*
     Definition of sockaddr_in struct
     
     struct sockaddr_in
     {
     short   sin_family;  must be AF_INET
     u_short sin_port;
     struct  in_addr sin_addr;
     char    sin_zero[8];  Not used, must be zero
     };
     */
}

void sendToAWS(char* mapID, char* vertexIndex, char* fileSize){
    
    string buf;
    
    
    if (send(aws_TCP_sockfd, mapID, strlen(mapID), 0) == -1){
        perror("Error sending data to server socket");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 10000000; i++){}
    if (send(aws_TCP_sockfd, vertexIndex, strlen(vertexIndex), 0) == -1){
        perror("Error sending data to server socket");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 10000000; i++){}
    if (send(aws_TCP_sockfd, fileSize, strlen(fileSize), 0) == -1){
        perror("Error sending data to server socket");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    
    // get dynamic client port number
    int client_portNum;
    socklen_t len = sizeof(clientAddr);
    if (getsockname(aws_TCP_sockfd, (struct sockaddr *)&clientAddr, &len) == -1){
        perror("Error getting client port number");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    else
        client_portNum = ntohs(clientAddr.sin_port);
    
    // Print message of data sent to AWS
    cout << "The client has sent query to AWS using TCP over port: " << client_portNum << " start vertex " << vertexIndex << "; map: " << mapID << "; file size: " << fileSize << "." << endl;
    
}

void recvFromAWS(){
    
    char destBuf [BUFLEN];
    char lenBuf[BUFLEN];
    char transBuf [BUFLEN];
    char propBuf [BUFLEN];
    char totBuf [BUFLEN];
    int recvLen1;
    
    // recv transmission delay
    if ( (recvLen1 = recv(aws_TCP_sockfd, transBuf, BUFLEN, 0)) == -1){
        perror("Error receiving message from AWS");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    transBuf[recvLen1] = '\0';
    
    // if buf == -2 then the map or source node is incorrect and client should abort
    if (atoi(transBuf) == -2){
        cerr << "Received error from AWS: Map or source node invalid\n";
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    else if (atoi(transBuf) == -1){
        cerr << "Received error from AWS: File size too large\n";
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    
    transDelay = atof(transBuf);
    //    cout << "DEBUG: " << transDelay << endl;
    
    
    int recvDone = 0; // 0 = not finished receiving, 1 = finished receiving
    while (!recvDone){
        // recv destination node (OR NULL char on last recv)
        if ( (recvLen1 = recv(aws_TCP_sockfd, destBuf, BUFLEN, 0)) == -1){
            perror("Error receiving message from AWS");
            close(client_sockfd);
            exit(EXIT_FAILURE);
        }
        destBuf[recvLen1] = '\0';
//    cout << "DEBUG dest: " << destBuf << endl;
        
        // receive propagation delay and total delay if NULL terminator not receieved
        if (destBuf[0] != '\0'){
            
            // recv destination length
            if ( (recvLen1 = recv(aws_TCP_sockfd, lenBuf, BUFLEN, 0)) == -1){
                perror("Error receiving message from AWS");
                close(client_sockfd);
                exit(EXIT_FAILURE);
            }
            lenBuf[recvLen1] = '\0';
//    cout << "DEBUG dest: " << lenBuf << endl;
            
            //recv prop delay
            if ( (recvLen1 = recv(aws_TCP_sockfd, propBuf, BUFLEN, 0)) == -1){
                perror("Error receiving message from AWS");
                close(client_sockfd);
                exit(EXIT_FAILURE);
            }
            propBuf[recvLen1] = '\0';
            //           cout << "DEBUG prop: " << propBuf << endl;
            
            //recv total delay
            if ( (recvLen1 = recv(aws_TCP_sockfd, totBuf, BUFLEN, 0)) == -1){
                perror("Error receiving message from AWS");
                close(client_sockfd);
                exit(EXIT_FAILURE);
            }
            totBuf[recvLen1] = '\0';
            //            cout << "DEBUG tot: " << totBuf << endl;
            
            // add destination, length , propagation delay, total delay to vector
            
            shortestPathPairs.push_back(make_pair(atoi(destBuf), atoi(lenBuf)) );
            propDelay.push_back(atof(propBuf));
            totDelay.push_back(atof(propBuf) + transDelay);
            
        }
        else{
            //toggle recvDone
            recvDone = 1;
        }
        
        //        cout << "DEBUG: " << recvDone << endl;
    } // end while
    
    
}

int main(int argc, const char * argv[]) {
    
    /*
     example client input
     ./client <Map ID> <Source Vertex Index> <File Size>
     */
    
    /* ADD VALID INPUT CHECKING */
    
    //argv[1] = map id
    //argv[2] = source node
    //argv[3] = file size
    
    // check for correct format
    if (argc != 4){
        cout << "Please input in the following format: " << endl << "'./client <Map ID> <Source Vertex Index> <File Size>'" << endl;
        exit(EXIT_FAILURE);
    }
    // check if map ID is letter
    if (!isalpha(*argv[1])){
        cout << "Map ID must be a letter" << endl;
        exit(EXIT_FAILURE);
    }
    // check if source and file are numbers
    if (!(isdigit(*argv[2]) && isdigit(*argv[3]))){
        cout << "Source vertex index and file size must be numerical digits" << endl;
        exit(EXIT_FAILURE);
    }
    // check if file size is in range that can be handled (long long)
    if(atoll(argv[3]) > 9223372036854775805){
        cout << "File size must fit in long long (0 - 9223372036854775805)" << endl;
        exit(EXIT_FAILURE);
    }
    
    init_TCP();
    cout << "The client is up and running." << endl;
    
    char mapID[BUFLEN];
    //    strcpy(mapID,argv[1]);
    sprintf(mapID,"%s",argv[1]);
    
    char vertexIndex[BUFLEN];
    //    strcpy(vertexIndex,argv[2]);
    sprintf(vertexIndex,"%s",argv[2]);
    
    char fileSize[BUFLEN]; // in bits
    //    strcpy(fileSize,argv[3]);
    sprintf(fileSize,"%s",argv[3]);
      
    /*
     Sources for setting up socket:
     Eduonix Learning Solutions: https://www.youtube.com/watch?v=LtXEMwSG5-8
     CMU 441 Computer Networks - Course powerpoint
     https://en.wikibooks.org/wiki/C_Programming/Networking_in_UNIX
     */
    
    
    // *** SEND DATA TO AWS ***
    // int send(int sockfd, const void *msg, int len, int flags);
    sendToAWS(mapID, vertexIndex, fileSize);
    
    // *** RECEIVE DATA FROM AWS AND PRINT RESULT ***
    recvFromAWS();
   
     cout << "The client has received results from AWS:" << endl;
     cout << "---------------------------------------------------------------------------------------------------" << endl;
     cout << left << setw(15) <<"Destination" <<setw(15) << "Min Length (km)" << setw(20) << "Tt" << setw(20) << "Tp" << setw(20) << "Delay (sec)" << endl;
    cout << "---------------------------------------------------------------------------------------------------" << endl;
    
    cout << fixed << setprecision(2);
    
    for (int i = 0; i < shortestPathPairs.size(); i++){
        cout << left << setw(15) << shortestPathPairs[i].first << setw(15) << shortestPathPairs[i].second << setw(20) << transDelay << setw(20) << propDelay[i] << setw(20) << totDelay[i] << endl;
    }
    
    cout << "---------------------------------------------------------------------------------------------------" << endl;
 
    // *** CLOSE CONNECTION ***
    shortestPathPairs.clear();
    propDelay.clear();
    totDelay.clear();
    
    close(client_sockfd);
    
    return 0;
}
