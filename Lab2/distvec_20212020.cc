#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <algorithm>

using namespace std;

struct Edge {
    int destination;
    int cost;
};

struct RoutingEntry {
    int destination;
    int distance;
    int nextHop;
};

void readTopology(const string& filename, vector<vector<Edge> >& graph, int& numNodes) {
    ifstream file(filename.c_str());
    if (!file) {
        cerr << "Error: open input file.\n";
        exit(EXIT_FAILURE);
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

void bellmanFord(const vector<vector<Edge> >& graph, int start, vector<RoutingEntry>& routingTable) {
    int n = graph.size();
    vector<int> distances(n, INT_MAX);
    vector<int> nextHops(n, -1);
    distances[start] = 0;

    for (int i = 0; i < n - 1; ++i) {
        for (int u = 0; u < n; ++u) {
            if (distances[u] == INT_MAX) continue;
            for (size_t j = 0; j < graph[u].size(); ++j) {
                const Edge& edge = graph[u][j];
                int v = edge.destination;
                int newDist = distances[u] + edge.cost;
                if (newDist < distances[v]) {
                    distances[v] = newDist;
                    nextHops[v] = u;
                } else if (newDist == distances[v] && u < nextHops[v]) {
                    nextHops[v] = u;
                }
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (distances[i] != INT_MAX) {
            int nextHop = i;
            while (nextHops[nextHop] != start && nextHops[nextHop] != -1) {
                int previousHop = nextHop;
                nextHop = nextHops[nextHop];
                if (previousHop == nextHop) {
                    nextHop = -1;
                    break;
                }
            }
            if (nextHop != -1) {
                routingTable.push_back((RoutingEntry){i, distances[i], nextHop});
            }
        }
    }
}

void writeRoutingTables(const vector<vector<RoutingEntry> >& routingTables, const string& filename) {
    ofstream out(filename.c_str(), ios_base::app);

    for (size_t i = 0; i < routingTables.size(); i++) {
        for (size_t j = 0; j < routingTables[i].size(); j++) {
            const RoutingEntry& entry = routingTables[i][j];
            out << entry.destination << ' ' << entry.nextHop << ' ' << entry.distance << '\n';
        }
        out << '\n';
    }

    out.close();
}

void handleMessages(const vector<vector<RoutingEntry> >& routingTables, const string& messagesFile, const string& outputFile) {
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
        int distance = routingTables[src][dest].distance;
        int currentNode = dest;
        
        if (distance == INT_MAX) {
            out << "from " << src << " to " << dest << " cost infinite hops unreachable message " << message << '\n';
        } 
        else {
            while (currentNode != src && currentNode != -1) {
                path.push_back(currentNode);
                currentNode = routingTables[currentNode][src].nextHop;
                if (currentNode == -1 || find(path.begin(), path.end(), currentNode) != path.end()) {
                    break;
                }
            }
            
            if (currentNode == -1 || currentNode != src) {
                out << "from " << src << " to " << dest << " cost infinite hops unreachable message " << message << '\n';
            } 
            else {
                path.push_back(src);
                out << "from " << src << " to " << dest << " cost " << distance << " hops ";
                for (int i = path.size() - 1; i > 0; i--) {
                    out << path[i] << ' ';
                }
                out << "message " << message << '\n';
            }
        }
    }
    out << '\n';
    
    file.close();
    out.close();
}

void applyChanges(vector<vector<Edge> >& graph, vector<vector<RoutingEntry> >& routingTables, const string& changesFile, const string& messagesFile, const string& outputFile) {
    ifstream file(changesFile.c_str());
    if (!file) {
        cerr << "Error: open input file.\n";
        return;
    }
    
    int u, v, cost;
    while (file >> u >> v >> cost) {
        for (vector<Edge>::iterator it = graph[u].begin(); it != graph[u].end();) {
            if (it->destination == v) {
                it = graph[u].erase(it);
            } else {
                ++it;
            }
        }
        
        for (vector<Edge>::iterator it = graph[v].begin(); it != graph[v].end();) {
            if (it->destination == u) {
                it = graph[v].erase(it);
            } else {
                ++it;
            }
        }
        
        if (cost != -999) {
            graph[u].push_back((Edge){v, cost});
            graph[v].push_back((Edge){u, cost});
        }

        routingTables.clear();
        routingTables.resize(graph.size());
        for (int i = 0; i < static_cast<int>(graph.size()); i++) {
            vector<RoutingEntry> routingTable;
            bellmanFord(graph, i, routingTable);
            routingTables[i] = routingTable;
        }
        
        writeRoutingTables(routingTables, outputFile);
        handleMessages(routingTables, messagesFile, outputFile);
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "usage: distvec topologyfile messagesfile changesfile\n";
        return 1;
    }
    
    string topologyFile = argv[1];
    string messagesFile = argv[2];
    string changesFile = argv[3];
    string outputFile = "output_dv.txt";
    
    vector<vector<Edge> > graph;
    int numNodes;
    readTopology(topologyFile, graph, numNodes);
    
    vector<vector<RoutingEntry> > routingTables(graph.size());

    for (int i = 0; i < static_cast<int>(graph.size()); i++) {
        vector<RoutingEntry> routingTable;
        bellmanFord(graph, i, routingTable);
        routingTables[i] = routingTable;
    }
    
    ofstream out(outputFile.c_str());
    writeRoutingTables(routingTables, outputFile);
    handleMessages(routingTables, messagesFile, outputFile);
    out.close();
    
    applyChanges(graph, routingTables, changesFile, messagesFile, outputFile);
    
    cout << "Complete. Output file written to output_dv.txt.\n";
    return 0;
}
