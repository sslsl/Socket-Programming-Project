
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <iostream>
#include<iomanip>
#include <vector>
#include <cstdlib>
#include <map>
#include <fstream>
#include <string>
#include <limits.h>
#include <float.h>
#include <stack>
#include <list>
#include <unordered_set>
#include <queue>

#include "def.h"

using namespace std;

int main(void)
{
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
	if ((res = getaddrinfo(HOST, PORTB, &hints, &server_info)) != 0) {
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

	cout<<"The server B is up and running using UDP on port "<<PORTB<<endl<<endl;
    while(1){

    
	addr_len = sizeof their_addr;
int point_count;
double prop_speed;
double tran_speed;
double file_size;
double tran_delay;
vector<pair <int,double> >  vector_distance;
vector< pair<double,double> >vector_prop_delay;
    numbytes = recvfrom(WHYsocket, &point_count, sizeof(point_count),0,p->ai_addr,&(p->ai_addrlen));
    if(numbytes == -1){
        perror("The AWS has received Destination from server A");
        exit(1);
    }
     numbytes = recvfrom(WHYsocket, &prop_speed, sizeof(prop_speed),0,p->ai_addr,&(p->ai_addrlen));
     numbytes = recvfrom(WHYsocket, &tran_speed, sizeof(tran_speed),0,p->ai_addr,&(p->ai_addrlen));
    numbytes = recvfrom(WHYsocket, &file_size, sizeof(file_size),0,p->ai_addr,&(p->ai_addrlen));
	cout<<"The Server B has received data for calculation:"<<endl;
	cout<<"* Propagation speed:   "<<prop_speed<<" km/s"<<endl; 
	cout<<"* Transmission speed:  "<<tran_speed<<"  Bytes/s"<<endl;
	cout<<"* File size:  "<<file_size<<"  bits"<<endl;
	tran_delay=(double)file_size/(tran_speed*8);
          int des;
        double dis;
	vector<int> vector_des;
    for(int i=0;i<point_count;++i){
           numbytes = recvfrom(WHYsocket, &des, sizeof(des),0,NULL,NULL);
           numbytes = recvfrom(WHYsocket, &dis, sizeof(dis),0,NULL,NULL);
		   cout<<"* Path length for destination "<<des<<" : "<<dis<<endl;
           //vector_distance.emplace_back (des,dis);
		   vector_prop_delay.emplace_back(dis/prop_speed,dis/prop_speed+tran_delay);
		   vector_des.emplace_back(des);
    }
	
	cout<<"The Server B has finished the calculation of  the delays:"<<endl;
	 cout << setfill('-') << setw(50) << '-' << endl;
     cout.fill(' '); 
    cout << left << setw(15) << "Destination" << setw(15)  << "Delay"<< setw(15) << endl;
	cout << setfill('-') << setw(50) << '-' << endl;
     cout.fill(' '); 
	 numbytes = sendto(WHYsocket, &point_count,sizeof(point_count),0,p->ai_addr,p->ai_addrlen);
	 numbytes = sendto(WHYsocket, &tran_delay,sizeof(tran_delay),0,p->ai_addr,p->ai_addrlen);
	 for(int i=0;i<point_count;++i){
		numbytes = sendto(WHYsocket, &(vector_prop_delay[i].first),sizeof(vector_prop_delay[i].first),0,p->ai_addr,p->ai_addrlen);
		numbytes = sendto(WHYsocket, &(vector_prop_delay[i].second),sizeof(vector_prop_delay[i].second),0,p->ai_addr,p->ai_addrlen);
		cout << left << setw(15) << vector_des[i]<< setw(15)  <<setprecision(2)<< vector_prop_delay[i].second<< setw(15) << endl;
	 }
	 cout << setfill('-') << setw(50) << '-' << endl;
     cout.fill(' '); 
	 cout<<"The Server B has finished sending the output to AWS"<<endl<<endl;
}
	close(WHYsocket);
	return 0;
}