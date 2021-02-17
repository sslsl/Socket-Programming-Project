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
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>
#include <climits>

//#include "serverA.h"
using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define UDPPORT 23984 // UDP Port # backend servers connects to
#define SERVERAPORT 21984
#define BUFLEN 1000 // Length of socket stream buffer

char buf [BUFLEN];
char recvMapID [BUFLEN];
char recvVertexIndex[BUFLEN];
char recvFileSize[BUFLEN];
int recvLen1;

struct sockaddr_in awsAddrUDP;

struct sockaddr_in serverAAddr;
int serverA_sockfd;

// structure to hold map.txt
struct graph{
    char mapID;
    double propSpeed; // in km/s
    double transSpeed; // in Bytes/s
    int numVert; // number of vertices in Map
    int numEdge; // number of edges in Map
    vector<vector<int> > adjmat;
    map<int,int> nodeMap; // (continuous index 1,2,..n , actual node index) (Key, Value)
    vector<int> node1;
    vector<int> node2;
    vector<int> edge; // distance in Km
    vector< pair <int, int> > shortestPathPairs; // (node, distance (km)) in ascending distance order
    
    void dijkstra(int source);
    int minDistance(int dist[], bool spt[]);
    void printDijkstra(vector< pair <int, int> > shortestPathPairs);
};
// vector of struct graph to hold all possibile mapIDs
vector<graph> graphs;


void init_UDP(){
    // *** 1. CREATE SOCKET ***
    if ( (serverA_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Error creating UDP socket");
        exit(EXIT_FAILURE);
    }
    
    // specify ServerA address
    
    serverAAddr.sin_family = AF_INET;
    //AWS Port #
    serverAAddr.sin_port = htons(SERVERAPORT);
    //AWS IP ADDR - INADDR_LOOPBACK refers to localhost ("127.0.0.1")
    serverAAddr.sin_addr.s_addr = inet_addr(LOCALIP);
    
    // *** 2. BIND SOCKET ***
    
    if (::bind(serverA_sockfd, (struct sockaddr *) &serverAAddr, sizeof(serverAAddr)) == -1 ){
        close(serverA_sockfd);
        perror("Error binding UDP socket");
        exit(EXIT_FAILURE);
    }
}

// Function to read map.txt, map index to node, and construct adjacency matrix.
void constructMap(){
    
    // Open the file "map.txt" for input
    std::ifstream fileInput("map.txt");
    if (fileInput.is_open()){
        
        // index of graphs struct
        int graphsIndex = 0;
        int nodeIndex = 0;
        string word;
        // Continue looping as long as not at EOF!
        while (fileInput.eof() != true) {
            // Use std::getline to grab a whole line
            /*
             std::string line;
             std::getline(fileInput, line);
             std::cout << line << std::endl;
             */
            if (graphsIndex == 0){
                
                fileInput >> word;
            }
            //cout << word << endl;
            
            // check if filestream is start of new MapID
            if ( isalpha(word.at(0)) ){
                nodeIndex = 0;
                graphs.push_back(graph());
                graphs[graphsIndex].mapID = word.at(0);
                //                cout << "Debug MapID: " << graphs[graphsIndex].mapID << endl;
                
                // store propagation speed
                fileInput >> graphs[graphsIndex].propSpeed;
                //                cout << "Debug PropSpeed: " << graphs[graphsIndex].mapID << ": " << graphs[graphsIndex].propSpeed << endl;
                
                // store Transmission speed
                fileInput >> graphs[graphsIndex].transSpeed;
                //                cout << "Debug TransSpeed: " << graphs[graphsIndex].mapID << ": " << graphs[graphsIndex].transSpeed << endl;
                
                // while fileInput != isalpha and != eof
                fileInput >> word;
                
                while(isalpha(word.at(0)) == false && fileInput.eof() == false ){
                    
                    graphs[graphsIndex].node1.push_back(stoi(word));
                    fileInput >> word;
                    graphs[graphsIndex].node2.push_back(stoi(word));
                    fileInput >> word;
                    graphs[graphsIndex].edge.push_back(stoi(word));
                    fileInput >> word;
                    
                    // For Debug only:
                    //                    cout << "Debug node1: " << graphs[graphsIndex].mapID << ": " << graphs[graphsIndex].node1[nodeIndex] << endl;
                    nodeIndex++;
                }
                
                
                
                graphsIndex++;
            } // end of MapID
            
            
            
        }
        
        // determine number of maps
        
        cout << "The Server A has constructed a list of " << graphs.size() << " maps: " << endl << "-------------------------------------------" << endl;
        
        
        vector<int> combinedNodes;
        vector<int>::iterator it;
        for (int i = 0; i < graphs.size(); i++){
            combinedNodes.resize(graphs[i].node1.size() + graphs[i].node2.size());
            copy(graphs[i].node1.begin(), graphs[i].node1.end(), combinedNodes.begin());
            copy(graphs[i].node2.begin(), graphs[i].node2.end(), combinedNodes.begin() + graphs[i].node1.size() );
            
            sort(combinedNodes.begin(),combinedNodes.end());
            it = unique(combinedNodes.begin(), combinedNodes.end());
            
            
            
            
            
            // determine number of edges in each map
            graphs[i].numEdge = graphs[i].edge.size();
            
            // vector of unique nodes
            combinedNodes.resize(distance(combinedNodes.begin(),it));
            
            // create map to index node numbers
            int mapKey = 0;
            for(it = combinedNodes.begin(); it < combinedNodes.end(); it++){
                graphs[i].nodeMap[mapKey] = *it;
                mapKey++;
            }
            
            // determine number of verticies in each map
            graphs[i].numVert = combinedNodes.size();
            
            
            // create size of adjacency matrix such that:
            // row = node1, col = node2
            graphs[i].adjmat.resize(graphs[i].numVert, vector<int>(graphs[i].numVert));
            
            
            int node1MapKey = -1;
            int node2MapKey = -1;
            //loop through nodes and construct adjacency matrix
            for (int j = 0; j < graphs[i].edge.size(); j++) {
                // find Key of mapped node value
                for (auto it = graphs[i].nodeMap.begin(); it != graphs[i].nodeMap.end(); it++){
                    if (it->second == graphs[i].node1[j]){
                        node1MapKey = it->first;
                    }
                    if (it->second == graphs[i].node2[j]){
                        node2MapKey = it->first;
                    }
                }
                graphs[i].adjmat[node1MapKey][node2MapKey] = graphs[i].edge[j];
                graphs[i].adjmat[node2MapKey][node1MapKey] = graphs[i].edge[j];
                // }
            }
            
            combinedNodes.clear();
        } // end of for iterating through each MapID
        
        cout << "Map ID   Num Vertices   Num Edges" << endl << "-------------------------------------------" << endl;
        for (int i = 0; i < graphs.size(); i++){
            cout << graphs[i].mapID << setw(15) << graphs[i].numVert << setw(15) << graphs[i].numEdge << endl;
        }
        
        cout << "-------------------------------------------" << endl;
        // a = 97, z = 122
        // A = 65, Z = 90
        
        
        //DEBUG: Prints Adjacency Matrix *******
        /*
         int flag = 1;
         for( auto it2 = graphs[2].nodeMap.begin(); it2 !=  graphs[2].nodeMap.end(); it2++){
         if (flag){
         cout <<  setw(8) << it2->second;
         flag = 0;
         }
         else
         cout <<  setw(4) << it2->second;
         }
         cout << endl << endl;
         
         map<int, int>::iterator it1 = graphs[2].nodeMap.begin();
         for( int i = 0; i < graphs[2].numVert; i++){
         cout << setw(4) << it1->second;
         for( int j = 0; j < graphs[2].numVert; j++){
         cout <<  setw(4) << graphs[2].adjmat[i][j];
         }
         cout << endl;
         
         it1++;
         }
         */
        
        
    } // end of reading file
    
    else {
        std::cout << "Error: File not found :(" << std::endl;
    }
    
    
} // end of construct map function

// Function to recv Map ID and Vertex Index respectively from AWS
void recvFromAWS(){
    socklen_t awsLen = sizeof(awsAddrUDP);
    //    recv map ID
    if ((recvLen1 = recvfrom(serverA_sockfd, recvMapID, BUFLEN, 0, (struct sockaddr *) &awsAddrUDP, &awsLen)) < 1){
        perror("Error receiving from AWS");
        exit(EXIT_FAILURE);
    }
    
    //    recv source vertex index
    if ((recvLen1 = recvfrom(serverA_sockfd, recvVertexIndex, BUFLEN, 0, (struct sockaddr *) &awsAddrUDP, &awsLen)) < 1){
        perror("Error receiving from AWS");
        exit(EXIT_FAILURE);
    }
    
    cout << "The Server A has received input for finding shortest paths: starting vertex " << recvVertexIndex << " of map " << recvMapID << "." << endl;
}


// determines min distance of nodes not in SPT from https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
int graph::minDistance(int dist[], bool spt[]){
    // Initialize min value
    int min = INT_MAX;
    int min_node;
    
    for (int v = 0; v < numVert; v++){
        // compare distance recorded in distance vector with
        if (spt[v] == false && dist[v] <= min){
            min = dist[v];
            min_node = v;
        }
    }
    return min_node;
}

// Prints shortest path to terminal
void graph::printDijkstra(vector< pair <int, int> > shortestPathPairs)
{
    
    cout << "The Server A has identified the following shortest paths:" << endl;
    cout << "-----------------------------------\n" << left << setw(20) << "Destination" << "Min Length\n-----------------------------------" << endl;
    
    for ( auto it = shortestPathPairs.begin(); it != shortestPathPairs.end(); it++){
        cout << left << setw(20) << nodeMap[it->first] << it->second << endl;
    }
    cout << "-----------------------------------" << endl;
}


// Dijkstra algorithm resource: https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
// and EE 450 class notes
void graph::dijkstra(int source){
    
    
    int dist[numVert]; // shortest distance from source to each node such that dist[i] = d -> shortest path from source to i = d
    
    // spanning tree algoritm - SPT
    bool spt[numVert]; // graph to create spanning tree by including all nodes in original map
    
    // Initialize all distances as INFINITE and spt[] as false
    //    INT_MAX ~= INF
    for (int i = 0; i < numVert; i++){
        dist[i] = INT_MAX;
        spt[i] = false;
    }
    
    int sourceKey = -1;
    // convert source node input to key so it corresponds to adjacency matrix
    for (auto it = nodeMap.begin(); it != nodeMap.end(); it++){
        if (it->second == source){
            sourceKey = it->first;
        }
    }
    if (sourceKey == -1){
        cout << "Source node not found in Map" << endl;
        // send error to AWS
        int sendLen;
        if ( ( sendLen = sendto(serverA_sockfd, "-2", strlen("-2"), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server A");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }
    
    // set source distance to 0 - not considered in output
    dist[sourceKey] = 0;
    
    // Find shortest path for all vertices
    for (int count = 0; count < numVert - 1; count++) {
        // check for min distances and store
        int node1 = minDistance(dist, spt);
        
        // add node1 in SPT
        spt[node1] = true;
        
        // Update dist value of the adjacent vertices of the picked vertex.
        for (int node2 = 0; node2 < numVert; node2++){
            
            // Update dist[node2] only if is not in spt, there is an edge from
            // node1 to node2, and total weight of path from src to node2 through node1 is
            // smaller than current value of dist[node2]
            if (!spt[node2] && adjmat[node1][node2] && dist[node1] != INT_MAX && dist[node1] + adjmat[node1][node2] < dist[node2]){
                
                dist[node2] = dist[node1] + adjmat[node1][node2];
            }
        }
    }
    
    
    // add shortest path elements in pair to reorder by ascending distance
    for (int i = 0; i < numVert; i++){
        shortestPathPairs.push_back( make_pair(i ,dist[i]) );
    }
    
    /*
    //lambda funct for sorting pairs by distance instead of vertex
    std::sort(shortestPathPairs.begin(), shortestPathPairs.end(), [](const std::pair<int,int> &left, const std::pair<int,int> &right) {
        return left.second < right.second;
    });
    */
    
    // erase source node from list
    shortestPathPairs.erase(shortestPathPairs.begin() + sourceKey);
    
    // print shortest path
    printDijkstra(shortestPathPairs);
}

// function to send shortest path data to AWS
// Sends Propagation speed, transmission speed followed by a loop of destination node and distance until the list is exhausted. (all items sent respectively)
void sendToAws(int graphIndex){
    char buf [BUFLEN];
    char destBuf[BUFLEN];
    char lenBuf[BUFLEN];
    int sendLen;
    
    sprintf(buf,"%f",graphs[graphIndex].propSpeed);
    
    // send prop speed to AWS
    if ( ( sendLen = sendto(serverA_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to AWS from Server A");
        exit(EXIT_FAILURE);
    }
    
    sprintf(buf,"%f",graphs[graphIndex].transSpeed);
    
    // send trans speed to AWS
    if ( ( sendLen = sendto(serverA_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to AWS from Server A");
        exit(EXIT_FAILURE);
    }
    
    memset(buf, '\0', sizeof(buf));
    // send destination and min length to AWS
    for ( auto it = graphs[graphIndex].shortestPathPairs.begin(); it != graphs[graphIndex].shortestPathPairs.end(); it++){
        
        
        
        // store destination node in buffer to send
        sprintf(destBuf,"%d",graphs[graphIndex].nodeMap[it->first]);
        // store min length in buffer to send
        sprintf(lenBuf,"%d",it->second);
        
        
        
        if ( ( sendLen = sendto(serverA_sockfd, destBuf, strlen(destBuf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server A");
            exit(EXIT_FAILURE);
        }
        // erase destBuf
        memset(destBuf, '\0', sizeof(destBuf));
        if ( ( sendLen = sendto(serverA_sockfd, lenBuf, strlen(lenBuf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Error sending UDP message to AWS from Server A");
            exit(EXIT_FAILURE);
        }
        // erase lenBuf
        memset(lenBuf, '\0', sizeof(lenBuf));
    }
    memset(buf, '\0', sizeof(buf));
    // Send NULL char to signify end of communication
    if ( ( sendLen = sendto(serverA_sockfd, buf, strlen(buf), 0, (struct sockaddr *) &awsAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Error sending UDP message to AWS from Server A");
        exit(EXIT_FAILURE);
    }
    
    
    
    cout << "The Server A has sent shortest paths to AWS." << endl;
    
    // Erase ordered pairs vector so it can be reused if there is another query
    graphs[graphIndex].shortestPathPairs.clear();
}

int main (){
    
    init_UDP();
    
    // ServerA boot up message
    cout << "The Server A is up and running using UDP on port " << SERVERAPORT << "." << endl;
    
    constructMap();
    
    //    recv Map ID & start Node
    //    int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);
    
    while(1){
        recvFromAWS();
        //int graphIndex = (int)(size_t)toupper(recvMapID[0]) - 65;
        int graphIndex = 0;
        for (int i = 0; i < graphs.size(); i++){
            if (graphs[i].mapID == recvMapID[0])
                graphIndex = i;
        }
        //cout << testIndex << endl;
        //cout << graphs[(int)(size_t)recvMapID - 65].mapID << endl;
        graphs[graphIndex].dijkstra(atoi(recvVertexIndex));
        //send result back to aws
        sendToAws(graphIndex);
        
    }
    
    return EXIT_SUCCESS;
}



/*
 <Map ID 1>
 <Propagation speed>
 <Transmission speed>
 <Vertex index for one end of the edge> <Vertex index for the other end> <Distance between the two vertices>
 … (Specification for other edges)
 <Map ID 2>
 */