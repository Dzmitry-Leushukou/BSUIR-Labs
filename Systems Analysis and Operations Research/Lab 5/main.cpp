#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

const long long NEGATIVE_INFINITY = numeric_limits<long long>::lowest() / 4;

struct Edge {
    int from;
    int to;
    long long length;
};

struct Graph {
    int vertexCount = 0;
    int edgeCount = 0;
    int source = 0;
    int target = 0;
    vector<Edge> edges;
    vector<vector<int>> outgoing;
    vector<vector<int>> incoming;
};

Graph readGraph(istream& input) {
    Graph graph;
    if (!(input >> graph.vertexCount >> graph.edgeCount))
        throw runtime_error("Input error: expected vertex and edge counts (n m).");
    if (graph.vertexCount <= 0 || graph.edgeCount < 0)
        throw runtime_error("Input error: n must be positive and m must be non-negative.");

    if (!(input >> graph.source >> graph.target))
        throw runtime_error("Input error: expected source and target vertices (s t).");
    if (graph.source < 1 || graph.source > graph.vertexCount ||
        graph.target < 1 || graph.target > graph.vertexCount)
        throw runtime_error("Input error: source and target must be in the range 1..n.");
    if (graph.source == graph.target)
        throw runtime_error("Input error: source and target vertices must be different.");

    --graph.source;
    --graph.target;
    graph.outgoing.assign(graph.vertexCount, {});
    graph.incoming.assign(graph.vertexCount, {});
    graph.edges.reserve(graph.edgeCount);

    for (int i = 0; i < graph.edgeCount; ++i) {
        int from, to;
        long long length;
        if (!(input >> from >> to >> length))
            throw runtime_error("Input error: every arc must contain from, to and length.");
        if (from < 1 || from > graph.vertexCount || to < 1 || to > graph.vertexCount)
            throw runtime_error("Input error: arc endpoint is outside the range 1..n.");
        if (length < 0)
            throw runtime_error("Input error: arc lengths must be non-negative.");

        --from;
        --to;
        graph.edges.push_back({from, to, length});
        graph.outgoing[from].push_back(i);
        graph.incoming[to].push_back(i);
    }

    string extra;
    if (input >> extra)
        throw runtime_error("Input error: unexpected data after the arc list.");
    return graph;
}

vector<int> topologicalOrder(const Graph& graph) {
    vector<int> indegree(graph.vertexCount, 0);
    for (const Edge& edge : graph.edges) ++indegree[edge.to];

    priority_queue<int, vector<int>, greater<int>> ready;
    for (int vertex = 0; vertex < graph.vertexCount; ++vertex)
        if (indegree[vertex] == 0) ready.push(vertex);

    vector<int> order;
    order.reserve(graph.vertexCount);
    while (!ready.empty()) {
        int vertex = ready.top();
        ready.pop();
        order.push_back(vertex);
        for (int edgeIndex : graph.outgoing[vertex]) {
            const Edge& edge = graph.edges[edgeIndex];
            if (--indegree[edge.to] == 0) ready.push(edge.to);
        }
    }

    if ((int)order.size() != graph.vertexCount)
        throw runtime_error("Input error: the graph contains a directed cycle.");
    return order;
}

void printPath(const vector<int>& path) {
    for (int i = 0; i < (int)path.size(); ++i) {
        if (i) cout << " -> ";
        cout << path[i] + 1;
    }
    cout << '\n';
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string filename;
    for (int i = 1; i < argc; ++i) {
        string argument = argv[i];
        if (!filename.empty()) {
            cerr << "Usage: " << argv[0] << " [input_file]\n";
            return 2;
        }
        filename = argument;
    }

    ifstream inputFile;
    if (!filename.empty()) {
        inputFile.open(filename);
        if (!inputFile) {
            cerr << "Cannot open input file: " << filename << '\n';
            return 2;
        }
    }
    istream& input = filename.empty() ? cin : inputFile;

    try {
        Graph graph = readGraph(input);
        vector<int> order = topologicalOrder(graph);

        vector<int> position(graph.vertexCount, -1);
        for (int i = 0; i < graph.vertexCount; ++i)
            position[order[i]] = i;

        int sourcePosition = position[graph.source];
        int targetPosition = position[graph.target];

        vector<long long> opt(graph.vertexCount, NEGATIVE_INFINITY);
        vector<int> predecessor(graph.vertexCount, -1);
        opt[graph.source] = 0; // (1) OPT(v_k) = 0.

        if (sourcePosition <= targetPosition) {
            // (2) OPT(v_i) = max(OPT(v_t) + l(v_t, v_i)) over incoming arcs.
            for (int orderIndex = sourcePosition + 1;
                 orderIndex <= targetPosition; ++orderIndex) {
                int vertex = order[orderIndex];
                for (int edgeIndex : graph.incoming[vertex]) {
                    const Edge& edge = graph.edges[edgeIndex];
                    if (opt[edge.from] == NEGATIVE_INFINITY) continue;
                    long long candidate = opt[edge.from] + edge.length;
                    if (candidate > opt[vertex]) {
                        opt[vertex] = candidate;
                        predecessor[vertex] = edge.from; // x(v_i).
                    }
                }
            }
        }

        cout << "Longest path in a directed acyclic graph\n";
        cout << "==========================================\n";
        cout << "Vertices: " << graph.vertexCount << ", arcs: " << graph.edgeCount << '\n';
        cout << "Source: v" << graph.source + 1 << ", target: v" << graph.target + 1 << '\n';
        cout << "Topological order: ";
        for (int i = 0; i < (int)order.size(); ++i) {
            if (i) cout << ' ';
            cout << order[i] + 1;
        }
        cout << '\n';
        if (sourcePosition > targetPosition) {
            cout << "\nSince k > l, target v" << graph.target + 1
                 << " is before source v" << graph.source + 1
                 << " in the topological order, so it is unreachable.\n";
            return 0;
        }

        cout << "\nForward pass (OPT(v_i), x(v_i)):\n";
        for (int orderIndex = sourcePosition; orderIndex <= targetPosition; ++orderIndex) {
            int vertex = order[orderIndex];
            cout << "OPT(v" << vertex + 1 << ") = ";
            if (opt[vertex] == NEGATIVE_INFINITY) cout << "-inf";
            else cout << opt[vertex];
            if (vertex == graph.source) {
                cout << ", x(v" << vertex + 1 << ") = -";
            } else if (predecessor[vertex] != -1) {
                cout << ", x(v" << vertex + 1 << ") = v" << predecessor[vertex] + 1;
            } else {
                cout << ", x(v" << vertex + 1 << ") = -";
            }
            cout << '\n';
        }

        if (opt[graph.target] == NEGATIVE_INFINITY) {
            cout << "\nResult: vertex v" << graph.target + 1
                 << " is unreachable from v" << graph.source + 1 << ".\n";
            return 0;
        }

        vector<int> path;
        for (int vertex = graph.target; vertex != -1; vertex = predecessor[vertex])
            path.push_back(vertex);
        reverse(path.begin(), path.end());

        cout << "\nResult:\n";
        cout << "Backward pass: ";
        for (int i = (int)path.size() - 1; i >= 0; --i) {
            if (i != (int)path.size() - 1) cout << " -> ";
            cout << path[i] + 1;
        }
        cout << "\nLongest path: ";
        printPath(path);
        cout << "Path length: " << opt[graph.target] << '\n';
    } catch (const exception& error) {
        cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

/*
Input format:
  n m
  s t
  from_1 to_1 length_1
  ...
  from_m to_m length_m

Vertices are numbered from 1 to n. Arc lengths are non-negative integers.
Example:
  6 7
  1 6
  1 2 5
  1 3 3
  2 4 4
  3 4 2
  2 5 2
  4 6 3
  5 6 7
*/
