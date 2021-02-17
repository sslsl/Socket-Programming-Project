#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <map>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <float.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <regex>
#include <limits.h>
#include <stack>
#include <list>
#include <unordered_set>
#include <queue>


#define MAXVEX 10
#define PORTA "21595" 
#define AWSPORT "24595"   //aws TCP port
#define UDPPORT "23595"     //UDP port
#define MAXBUFLEN 100
#define HOST "localhost"

using namespace std;



typedef int previous[MAXVEX];
typedef int shorest_distance[MAXVEX];

typedef struct info{
    int vex[MAXVEX];
    int graphmatrix[MAXVEX][MAXVEX];
    int numvex;
    bool visit;

}WHYGraph;


//beej
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


class graph{

public:
    char map_id;
    double prop_speed;
    double tran_speed;
    int weight;
    int vex[MAXVEX];
    int SPT[MAXVEX];
    int graphmatrix[MAXVEX][MAXVEX];
    int previous;
    int num_vertices;
    int num_edges;

public:

    int create_graph(int v1,int v2,int weight){
        int i = v1;
        int j = v2;
        for (i=0;i<MAXVEX;i++){
            vex[i] = i;
        }
        for (i =0; i<10; i++){
            for (j=0; j<10; j++){
                if(i==j){
                    graphmatrix[i][j] = 0;
                }
                else{
                    graphmatrix[i][j] = graphmatrix[j][i] = weight;
                }

            }
        }
        return (graphmatrix[v1][v2]);
    }
    void Dijkstra(WHYGraph G, int v0,int Previous[], shorest_distance D ){
        int SPT[MAXVEX];
        int i;
        int min = -1;
        for(i=0;i<MAXVEX;i++){
            SPT[i] = 0;
            D[i] = graphmatrix[v0][i];
            Previous[i] = i;
        }
        D[v0] = 0;
        for(i=0;i<MAXVEX;i++){
            int j,k,w;
            for (j=0;j<MAXVEX;j++){
                if (!SPT[j]&&D[j]<min){
                    k=j;
                    min=D[j];
                }
            }
            SPT[k] = 1;
            for(w=0;w<MAXVEX;w++){
                if(!SPT[w]&&(min+G.graphmatrix[k][w])<D[w]){
                    D[w] = min + G.graphmatrix[k][w];
                    Previous[w] = k;
                }
            }
        }
    }
};

typedef struct p2p{
    int begin_p;
    int end_p;
    double p2p_weight;

}p2p_dist;

class Cmap{

public:
    char map_id;
    double prop_speed;
    double tran_speed;

    //double ** graphmatrix;
    unordered_set<int> points;
    vector<p2p_dist> weight;
    int point_count;
    vector<vector<double>> adjMatrix;

    Cmap(){};
    ~Cmap(){};
    Cmap(char id, double prop, double tran):map_id(id), prop_speed(prop), tran_speed(tran){}

    // construct adjacency Matrix
    void creatGraph()
    {
        point_count = points.size();
        adjMatrix.reserve(point_count);
        for(int i=0; i<point_count; i++)
        {
            adjMatrix.emplace_back(point_count, 0);
        }
        for(auto p : weight)
        {
            adjMatrix[p.begin_p][p.end_p] = p.p2p_weight;
            adjMatrix[p.end_p][p.begin_p] = p.p2p_weight;
        }
    }

    //find shortest paths
    typedef pair<int, int> pValue;
    vector<double> Dijkstra(int src)
    {
        vector<double> shortestPath(point_count, DBL_MAX);
        shortestPath[src] = 0;
        unordered_set<int> visited;
        priority_queue<pValue, vector<pValue>, greater<pValue>> pq;
        pq.emplace(0, src);
        while(!pq.empty())
        {
            int p = pq.top().second;
            pq.pop();
            if(visited.count(p)) continue;
            visited.insert(p);

            for(int i=0; i<point_count; i++)
            {
                if(adjMatrix[p][i] != 0)
                {
                    if(shortestPath[i] > shortestPath[p] + adjMatrix[p][i])
                    {
                        shortestPath[i] = shortestPath[p] + adjMatrix[p][i];
                        pq.emplace(shortestPath[i], i);
                    }
                }
            }
        }
        return shortestPath;
    }
/*
    void adjust_points(int x){
        std::vector<int>::iterator it;
        it=find(points.begin(),points.end(),x);
        if(it==points.end())
            points.push_back(x);
    }

    void get_point(){
        if(weight.size()!=0)
        for(int i=0;i<weight.size();i++)
            {
                adjust_points(weight.at(i).begin_p);
                adjust_points(weight.at(i).end_p);
            }

        if(points.size()!=0)
        point_count=points.size();
    }
*/

};



int main(int argc,char const *argv[]){
	int Num_Vertics,Num_Edges;
	char Map_ID;
    string file_txt = "map.txt";
    string buffer;
    vector <string> vs;
    vector <Cmap*> map_list;
    int map_num=0;


//******push all string**********
    ifstream inFile(file_txt, ios::in);
    while(getline(inFile, buffer))
    {
        vs.push_back(buffer);
        if(buffer.length() == 2)  map_num+=1;
    }
    inFile.close();
    

//*******Create Cmap*********
    
    vector<string>::iterator it;
    /*
    for(int i=0;i<map_num;i++)
    {
        Cmap*  temp_map =new Cmap();
        map_list.push_back(temp_map);
    }
    */
    int temp_map_id=-1;

    it=vs.begin();
    while(it<vs.end())
    {
        buffer=*it;
        it++;

        //*********First three ID Weight1 weight2***************
        if(buffer.length()==2)
        {
            temp_map_id+=1;
            
            char id = buffer[0];
            double prop = stod(*it++);
            double tran = stod(*it++);
            Cmap* tmp_map = new Cmap(id, prop, tran);
            map_list.push_back(tmp_map);

            //map_list[temp_map_id]->map_id=buffer[0];
            //map_list[temp_map_id]->prop_speed= stod(*it++);
            //map_list[temp_map_id]->tran_speed= stod(*it++);

            continue;
        }


        //******Add Normal Distance************
        const char* point = buffer.c_str();

        p2p_dist temp_p2p =p2p_dist();
        sscanf(point,"%d %d %lf",&temp_p2p.begin_p, &temp_p2p.end_p,&temp_p2p.p2p_weight);
        map_list[temp_map_id]->weight.push_back(temp_p2p);
        map_list[temp_map_id]->points.insert(temp_p2p.begin_p);
        map_list[temp_map_id]->points.insert(temp_p2p.end_p);
    }

    /*
    for (int i = 0;i<map_list.size();i++){
        for (int j = 0;j < map_list[i]->weight.size();j++){
            cout << map_list[i]->weight.at(j).begin_p << map_list[i]->weight.at(j).end_p << map_list[i]->weight.at(j).p2p_weight<<endl;
        }   
    }
    */
    for(Cmap* map : map_list) map->creatGraph(); 
    for(Cmap* map : map_list)
    {
        for(int p : map->points)
        {
            vector<double> res = map->Dijkstra(p);
            for(double dist : res)
            {
                cout<<dist<<" ";
            }
            cout<<endl;
        }
    }

    int WHYsocket;
	struct addrinfo hints, *server_info, *p;
	int res;
	int numbytes;
	char buf[MAXBUFLEN];
	
	struct sockaddr_storage their_addr;

	socklen_t addr_len;
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // 设 定 AF_INET 以 强 制 使 用 IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // 使 用 我 的 IP
	if ((res = getaddrinfo(HOST, UDPPORT, &hints, &server_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		return 1;
	}
// 用 循 环 找 出 全 部 的 结 果 , 并 bind 到 首 先 找 到 能 bind 的
	for(p = server_info; p != NULL; p = p->ai_next) {
		if ((WHYsocket = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(WHYsocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(WHYsocket);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(server_info);
	printf("The server A is up and running using UDP on port %s.\n",PORTA);


    while(1){
        //receive all the inforamtion from client
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(WHYsocket, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
		}
        printf("Accept Successfully!\n");
        recv(numbytes, &Map_ID, sizeof(Map_ID), 0);
        recv(numbytes, &Num_Vertics, sizeof(Num_Vertics), 0);
        recv(numbytes, &Num_Edges, sizeof(Num_Edges), 0);
        printf("The server A has constructed a list of %d maps:\n", map_num);//need to be filled in
        printf("Map ID   Num Vertics   Num Edges");

        printf("%s\n",Map_ID);
		recv()//need to be filled in
        printf("The server A has received input for finding shortest paths:starting vertex %d of map %s", vertex, Map_ID);

        close(socket_fd); 
      }

    return 0;
}



