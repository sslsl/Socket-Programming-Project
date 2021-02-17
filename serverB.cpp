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

#define PORTB "22595" 
#define AWSPORT "24595"   //aws TCP port
#define UDPPORT "23595"     //UDP port
#define MAXBUFLEN 100
#define HOST "localhost"
// get sockaddr, IPv4 or IPv6:

//beej
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


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
	vector<double> delay(vector<double> shortestPath){
		double File_size;
        vector<double> Tp;
        vector<double> Tt;
        vector<double> delay;
        for (int i = 0; i < shortestPath.size(); i++){
            Tp.push_back((double)prop_speed*shortestPath[i]);
            Tt.push_back((double)tran_speed*shortestPath[i]);
            for (int j = 0; j<shortestPath.size();j++){
                delay.push_back((double)Tp[j]+(double)Tt[j]);
            }

        }
        return delay;
    }
};


int main(int argc,char const *argv[])
{
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
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int res;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 

	if ((res = getaddrinfo(HOST, UDPPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	printf("The server B is up and running using UDP on port %s.\n",PORTB);
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	printf("listener: got packet from %s\n",
	inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);
	close(sockfd);
	return 0;
}
