
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
#include <iomanip>
 
 #include "dijkstra.h"
 #include "def.h"

int main(){
	string file_txt = "map.txt";
    string buffer;
    char Map_ID;
    int Source_Vertex_Index;
    vector <string> vs;
    vector <Cmap*> map_list;
    int map_num=0;
    


//******push all string**********
    ifstream inFile(file_txt, ios::in);
    while(getline(inFile, buffer))
    {
        vs.push_back(buffer);
		if (buffer <= "Z" && buffer >= "A") {
			++map_num;
		}
    }
    inFile.close();
    

//*******Create Cmap*********
    vector<string>::iterator it;
  
    int temp_map_id=-1;

    it=vs.begin();
    while(it<vs.end())
    {
        buffer=*it;
        it++;

        //*********First three ID Weight1 weight2***************
        if(buffer <= "Z" && buffer >= "A")
        {
           
            temp_map_id+=1;
           
            char id = buffer[0];
            double prop = stod(*it++);
            double tran = stod(*it++);
            Cmap* tmp_map = new Cmap(id, prop, tran);
            map_list.push_back(tmp_map);
			tmp_map->max_point = -1;
           
      
          
            continue;
        }


        //******Add Normal Distance************
        const char* point = buffer.c_str();

        p2p_dist temp_p2p =p2p_dist();
        sscanf(point,"%d %d %lf",&temp_p2p.begin_p, &temp_p2p.end_p,&temp_p2p.p2p_weight);
        map_list[temp_map_id]->weight.push_back(temp_p2p);
        map_list[temp_map_id]->points.insert(temp_p2p.begin_p);
        map_list[temp_map_id]->points.insert(temp_p2p.end_p);
		int temp = map_list[temp_map_id]->max_point;
		temp = temp>temp_p2p.begin_p?temp: temp_p2p.begin_p;
        temp = temp > temp_p2p.end_p? temp : temp_p2p.end_p;
		map_list[temp_map_id]->max_point= temp;
    }
    
   vector <pair<double,int> > pair_distance;
    for(Cmap* map : map_list) map->creatGraph(); 
    cout << "The Server A has constructed a list of   " << temp_map_id +1<<" maps:"<< endl;
	std::cout << setfill('-') << setw(50) << '-' << endl;
    std::cout.fill(' ');  
    cout<<setw(15)<<"Map ID"<<setw(15)<< "Num Vertices"<<setw(15)<<" Num Edges"<<endl;
    std::cout << setfill('-') << setw(50) << '-' << endl;
    std::cout.fill(' ');  
    for(int i=0;i<temp_map_id+1;i++){
    cout<<setw(15)<<map_list[i]->map_id<<setw(15)<< map_list[i]->point_count<<setw(15)<<map_list[i]->num_edges<<endl;
    }
    std::cout << setfill('-') << setw(50) << '-' << endl;
    std::cout.fill(' '); 
    int WHYsocket;
	struct addrinfo hints, *server_info, *p;
	int res;
	int numbytes;
	char buf[MAXBUFLEN];
	
	struct sockaddr_storage their_addr;

	// socklen_t addr_len;
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // 设 定 AF_INET 以 强 制 使 用 IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // 使 用 我 的 IP
	if ((res = getaddrinfo(HOST, PORTA, &hints, &server_info)) != 0) {
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

	// freeaddrinfo(server_info);
    
	
cout<<"The server A is up and running using UDP on port  "<<PORTA<<endl;
    while(1)
    {
        vector<pair<double ,int> >().swap(pair_distance);
        cout<<endl;
        //receive all the inforamtion from client
        // addr_len = sizeof their_addr;
        // if ((numbytes = recvfrom(WHYsocket, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		// perror("recvfrom");
		// exit(1);
		// }
    //    cout<<"Accept Successfully!"<<endl;
        // recv(numbytes, &Map_ID, sizeof(Map_ID), 0);
        // recv(numbytes, &Source_Vertex_Index, sizeof(Source_Vertex_Index), 0);
        

        numbytes = recvfrom(WHYsocket, &Map_ID, sizeof(Map_ID),0,p->ai_addr,&(p->ai_addrlen));
        if(numbytes == -1){
         perror("TThe ServerA failed receiving map_id from server AWS");
         exit(1);
        }
         numbytes = recvfrom(WHYsocket, &Source_Vertex_Index, sizeof(Source_Vertex_Index),0,p->ai_addr,&(p->ai_addrlen));
        if(numbytes == -1){
            perror("The ServerA failed receiving source_index from server AWS");
             exit(1);
         }
         

        cout << setfill('-') << setw(50) << '-' << endl;
        cout.fill(' ');         //print out the result and then close the socket!
        cout << left << setw(15) << "Map ID" << setw(15)  << "Num Vertics" << setw(15) << "Num Edges"<< setw(15) << endl;
        cout << setfill('-') << setw(50) << '-' << endl;
         cout.fill(' ');
       for (int i = 0;i < temp_map_id +1;i++) {
         cout << left << setw(15) <<map_list[i]->map_id<< setw(15) << map_list[i]->points.size()<< setw(15) << map_list[i]->num_edges;
            cout << left << fixed << setw(15) << setprecision(2) << endl;
        }

         double prop;
        double trans;
        for(Cmap* in :map_list){
            if(in->map_id==Map_ID){
               pair_distance=in->Dijkstra(Source_Vertex_Index);
               prop=in->prop_speed;
               trans=in->tran_speed;
               break;
            }
        }
    
        cout<<"The server A has received input for finding shortest paths:starting vertex "<<Source_Vertex_Index<<" of map "<< Map_ID<<endl<<endl;
        cout << setfill('-') << setw(50) << '-' << endl;
    	cout.fill(' ');   
    	cout << left << setw(15) << "Destination" << setw(15)  << "Min Length"<< setw(15) << endl;
        //sleep(5);
        cout << setfill('-') << setw(50) << '-' << endl;
        cout.fill(' ');
         int point_num=pair_distance.size();
        numbytes = sendto(WHYsocket, &point_num,sizeof(point_num),0,p->ai_addr,p->ai_addrlen);
        
            numbytes=sendto(WHYsocket,&prop,sizeof(prop),0,p->ai_addr,p->ai_addrlen);
            numbytes=sendto(WHYsocket,&trans,sizeof(trans),0,p->ai_addr,p->ai_addrlen);
        for(int i=0;i<pair_distance.size();i++){
            cout<<left<<setw(15)<<pair_distance[i].second<<setw(15)<<pair_distance[i].first<<setw(15)<<endl;
            int des=pair_distance[i].second;
            double dis=pair_distance[i].first;

            numbytes = sendto(WHYsocket, &des,sizeof(des),0,p->ai_addr,p->ai_addrlen);
            numbytes=sendto(WHYsocket,&dis,sizeof(dis),0,p->ai_addr,p->ai_addrlen);
    
        }
        cout << setfill('-') << setw(50) << '-' << endl;
        cout.fill(' '); 
        cout<<"The Server A has sent shortest paths to AWS."<<endl;
        cout<<endl;
        
      }
   close(WHYsocket); 
    return 0;
}

