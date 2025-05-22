#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <climits>
#include <cstring>
#include <algorithm>

using namespace std;

struct Edge {
    int destination;
    int cost;
};

struct Node {
    int id;
    int cost;
    bool operator>(const Node& other) const {
        return cost > other.cost;
    }
};

void dijkstra(const vector<vector<Edge> >& graph, int start, vector<int>& distances, vector<int>& parents) {
    int n = graph.size();
    distances.assign(n, INT_MAX);
    parents.assign(n, -1);
    distances[start] = 0;

    priority_queue<Node, vector<Node>, greater<Node> > pq;
    pq.push((Node){start, 0});

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (current.cost > distances[current.id]) continue;

        for (size_t i = 0; i < graph[current.id].size(); i++) {
            const Edge& edge = graph[current.id][i];
            int newDist = distances[current.id] + edge.cost;
            if (newDist < distances[edge.destination]) {
                distances[edge.destination] = newDist;
                parents[edge.destination] = current.id;
                pq.push((Node){edge.destination, newDist});
            } 
            else if (newDist == distances[edge.destination] && current.id < parents[edge.destination]) {
                parents[edge.destination] = current.id;
            }
        }
    }
}

void readTopology(const string& filename, vector<vector<Edge> >& graph, int& numNodes) {
    ifstream file(filename.c_str());
    if (!file) {
        cerr << "Error: open input file.\n";
        return;
    }

    file >> numNodes;
    graph.resize(numNodes);

    int u, v, cost;
    while (file >> u >> v >> cost) {
        graph[u].push_back((Edge){v, cost});
        graph[v].push_back((Edge){u, cost});
    }

    file.close();
}

void writeRoutingTables(const vector<vector<int> >& routingTables, const vector<vector<int> >& parents, const string& filename) {
    ofstream out(filename.c_str(), ios_base::app);

    for (size_t i = 0; i < routingTables.size(); i++) {
        for (size_t j = 0; j < routingTables[i].size(); j++) {
            if (routingTables[i][j] != INT_MAX && static_cast<int>(i) != static_cast<int>(j)) { 
                int nextHop = j;
                while (parents[i][nextHop] != static_cast<int>(i) && parents[i][nextHop] != -1) {
                    nextHop = parents[i][nextHop];
                }
                out << j << ' ' << nextHop << ' ' << routingTables[i][j] << '\n';
            } else if (static_cast<int>(i) == static_cast<int>(j)) {
                out << j << ' ' << i << ' ' << routingTables[i][j] << '\n';
            }
        }
        out << '\n';
    }

    out.close();
}

void handleMessages(const vector<vector<int> >& routingTables, const string& messagesFile, const string& outputFile, const vector<vector<int> >& parents) {
    ifstream file(messagesFile.c_str());
    if (!file) {
        cerr << "Error: open input file.\n";
        return;
    }

    ofstream out(outputFile.c_str(), ios_base::app);
    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        int src, dest;
        string message;
        iss >> src >> dest;
        getline(iss, message);
        message = message.substr(1);

        vector<int> path;
        int currentNode = dest;
        while (currentNode != src && currentNode != -1) {
            path.push_back(currentNode);
            currentNode = parents[src][currentNode];
        }

        if (currentNode == -1) {
            out << "from " << src << " to " << dest << " cost infinite hops unreachable message " << message << '\n';
        } 
        else {
            path.push_back(src);

            out << "from " << src << " to " << dest << " cost " << routingTables[src][dest] << " hops ";
            for (int i = path.size() - 1; i > 0; i--) {
                out << path[i] << ' ';
            }
            out << "message " << message << '\n';
        }
    }
    out << '\n';

    file.close();
    out.close();
}

void applyChanges(vector<vector<Edge> >& graph, vector<vector<int> >& routingTables, vector<vector<int> >& parents, const string& changesFile, const string& messagesFile, const string& outputFile) {
    ifstream file(changesFile.c_str());
    if (!file) {
        cerr << "Error: open input file.\n";
        return;
    }

    int u, v, cost;
    while (file >> u >> v >> cost) {
        for (vector<Edge>::iterator it = graph[u].begin(); it != graph[u].end();) {
            if (it->destination == v)
                it = graph[u].erase(it);
            else 
                it++;
        }

        for (vector<Edge>::iterator it = graph[v].begin(); it != graph[v].end();) {
            if (it->destination == u)
                it = graph[v].erase(it);
            else
                it++;
        }

        if (cost != -999) {
            graph[u].push_back((Edge){v, cost});
            graph[v].push_back((Edge){u, cost});
        }

        for (size_t i = 0; i < graph.size(); i++) {
            vector<int> distances, parent;
            dijkstra(graph, i, distances, parent);
            for (size_t j = 0; j < graph.size(); j++) {
                routingTables[i][j] = distances[j];
                parents[i][j] = parent[j];
            }
        }

        writeRoutingTables(routingTables, parents, outputFile);
        handleMessages(routingTables, messagesFile, outputFile, parents);
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "usage: linkstate topologyfile messagesfile changesfile\n";
        return 1;
    }
    
    string topologyFile = argv[1];
    string messagesFile = argv[2];
    string changesFile = argv[3];
    string outputFile = "output_ls.txt";
    
    vector<vector<Edge> > graph;
    int numNodes;
    readTopology(topologyFile, graph, numNodes);
    
    vector<vector<int> > routingTables(numNodes, vector<int>(numNodes, INT_MAX));
    vector<vector<int> > parents(numNodes, vector<int>(numNodes, -1));

    for (int i = 0; i < numNodes; i++) {
        vector<int> distances, parent;
        dijkstra(graph, i, distances, parent);
        for (int j = 0; j < numNodes; j++) {
            routingTables[i][j] = distances[j];
            parents[i][j] = parent[j];
        }
    }
    
    ofstream out(outputFile.c_str());
    writeRoutingTables(routingTables, parents, outputFile);
    handleMessages(routingTables, messagesFile, outputFile, parents);
    out.close();
    
    applyChanges(graph, routingTables, parents, changesFile, messagesFile, outputFile);

    cout << "Complete. Output file written to output_ls.txt.\n";
    return 0;
}
