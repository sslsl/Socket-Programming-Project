#include "dijkstra.h"




int   graph::create_graph(int v1,int v2,int weight){
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
    void graph::Dijkstra(WHYGraph G, int v0,int Previous[], shorest_distance D ){
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






    // construct adjacency Matrix
    void Cmap::creatGraph()
    {
        num_edges = weight.size();
        point_count = points.size();
	 
        adjMatrix.reserve(max_point);
        for(int i=0; i<=max_point; i++)
        {
            adjMatrix.emplace_back(max_point+1, -1);
        }
        for(auto p : weight)
        {
            adjMatrix[p.begin_p][p.end_p] = p.p2p_weight;
            adjMatrix[p.end_p][p.begin_p] = p.p2p_weight;
        }
    }

    //find shortest paths
    
    vector<pair<double,int> >Cmap::Dijkstra(int src)
    {
        vector <double> shortestPath(max_point+1, DBL_MAX);
        vector <pair<double,int>> minDistance;
        shortestPath[src] = 0;
        unordered_set<int> visited;
        priority_queue<pValue, vector<pValue>, greater<pValue> > pq;
        pq.emplace(0, src);
        while(!pq.empty())
        {
            int p = pq.top().second;
            pq.pop();
            if(visited.count(p)) continue;
            visited.insert(p);

            for(int i=1; i<=max_point; i++)
            {
                if(adjMatrix[p][i] != -1)
                {
                    if(shortestPath[i] > shortestPath[p] + adjMatrix[p][i])
                    {
                        shortestPath[i] = shortestPath[p] + adjMatrix[p][i];
                        pq.emplace(shortestPath[i], i);
                    }
                }
            }
        }
        for(int i = 0;i<=max_point;i++){
            if(shortestPath[i]<DBL_MAX-2&&i!=src){
                minDistance.push_back(make_pair(shortestPath[i],i));
            } 
        }
        sort(minDistance.begin(),minDistance.end());
        return minDistance;

    }




