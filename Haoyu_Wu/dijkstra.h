#pragma once
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
typedef pair<int, int> pValue;
typedef struct info {
	int vex[MAXVEX];
	int graphmatrix[MAXVEX][MAXVEX];
	int numvex;
	bool visit;

}WHYGraph;

typedef struct p2p {
	int begin_p;
	int end_p;
	double p2p_weight;

}p2p_dist;

class graph {

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
	int create_graph(int v1, int v2, int weight);
	void Dijkstra(WHYGraph G, int v0, int Previous[], shorest_distance D);
};



class Cmap {

public:
	char map_id;
	double prop_speed;
	double tran_speed;
	int num_edges;
	int max_point;

	//double ** graphmatrix;
	unordered_set<int> points;
	vector<p2p_dist> weight;
	int point_count;
	vector<vector<double>> adjMatrix;
	vector<pair<double ,int>  >shortDistance;

	Cmap() {};
	~Cmap() {};
	Cmap(char id, double prop, double tran) :map_id(id), prop_speed(prop), tran_speed(tran) {};
	void creatGraph();
	vector <pair<double, int>> Dijkstra(int src);
};