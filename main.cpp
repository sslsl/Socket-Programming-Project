#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <map>
#include <fstream>
#include <string>
#include <regex>
#include <limits.h>
#include <float.h>
#include <stack>
#include <list>
#include <unordered_set>
#include <queue>

using namespace std;

#define MAXVEX 10

typedef int previous[MAXVEX];
typedef int shorest_distance[MAXVEX];

typedef struct info{
    int vex[MAXVEX];
    int graphmatrix[MAXVEX][MAXVEX];
    int numvex;
    bool visit;

}WHYGraph;

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

    string file_txt = "map.txt";
    string buffer;
    vector <string> vs;
    vector <Cmap*> map_list;
    int map_num=0;
    vector <int> num_vertices;
    vector <int> num_edges;


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
        5
    }
    

}
