#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct BipartiteGraph {
    int leftSize = 0;
    int rightSize = 0;
    vector<vector<int>> adjacent;
};

class MaximumMatching {
    const BipartiteGraph& graph;
    vector<int> matchedLeft;
    vector<int> matchedRight;
    bool detailed;
    int iterations = 0;

    int rightVertex(int right) const { return graph.leftSize + right; }

    void printMatching() const {
        cout << "    Matching: ";
        bool hasEdges = false;
        for (int left = 0; left < graph.leftSize; ++left) {
            if (matchedLeft[left] == -1) continue;
            if (hasEdges) cout << ", ";
            cout << "(u" << left + 1 << ", v" << matchedLeft[left] + 1 << ")";
            hasEdges = true;
        }
        if (!hasEdges) cout << "empty";
        cout << '\n';
    }

    bool findAugmentingPath(vector<int>& parent, int& finish) const {
        const int source = graph.leftSize + graph.rightSize;
        const int sink = source + 1;
        const int vertexCount = sink + 1;
        parent.assign(vertexCount, -1);
        vector<bool> visited(vertexCount, false);
        queue<int> pending;

        visited[source] = true;
        pending.push(source);

        while (!pending.empty()) {
            int current = pending.front();
            pending.pop();

            if (current == source) {
                for (int left = 0; left < graph.leftSize; ++left) {
                    if (matchedLeft[left] == -1 && !visited[left]) {
                        visited[left] = true;
                        parent[left] = source;
                        pending.push(left);
                    }
                }
                continue;
            }

            if (current < graph.leftSize) {
                int left = current;
                for (int right : graph.adjacent[left]) {
                    if (matchedLeft[left] == right) continue;
                    int next = rightVertex(right);
                    if (visited[next]) continue;
                    visited[next] = true;
                    parent[next] = current;
                    pending.push(next);
                }
            } else if (current < source) {
                int right = current - graph.leftSize;
                if (matchedRight[right] == -1) {
                    if (!visited[sink]) {
                        visited[sink] = true;
                        parent[sink] = current;
                        finish = sink;
                        return true;
                    }
                } else {
                    int next = matchedRight[right];
                    if (!visited[next]) {
                        visited[next] = true;
                        parent[next] = current;
                        pending.push(next);
                    }
                }
            }
        }

        return false;
    }

    vector<int> pathWithoutTerminals(const vector<int>& parent, int finish) const {
        const int source = graph.leftSize + graph.rightSize;
        vector<int> reversed;
        // The sink itself is omitted; its parent is the final right vertex.
        for (int vertex = parent[finish]; vertex != source && vertex != -1; vertex = parent[vertex])
            reversed.push_back(vertex);
        reverse(reversed.begin(), reversed.end());
        return reversed;
    }

    void augment(const vector<int>& path) {
        // Remove the matched edges traversed backwards by the alternating path.
        for (size_t i = 1; i + 1 < path.size(); i += 2) {
            int right = path[i] - graph.leftSize;
            int left = path[i + 1];
            if (matchedRight[right] == left) {
                matchedRight[right] = -1;
                matchedLeft[left] = -1;
            }
        }

        // Add every forward edge of the path to the matching.
        for (size_t i = 0; i + 1 < path.size(); i += 2) {
            int left = path[i];
            int right = path[i + 1] - graph.leftSize;
            matchedLeft[left] = right;
            matchedRight[right] = left;
        }
    }

    void printPath(const vector<int>& path) const {
        cout << "    Augmenting path: s";
        for (int vertex : path) {
            if (vertex < graph.leftSize) cout << " -> u" << vertex + 1;
            else cout << " -> v" << vertex - graph.leftSize + 1;
        }
        cout << " -> t\n";
    }

    void printResidualGraph() const {
        cout << "    G* after correction:\n";
        cout << "      s ->";
        bool hasFreeLeft = false;
        for (int left = 0; left < graph.leftSize; ++left) {
            if (matchedLeft[left] == -1) {
                cout << " u" << left + 1;
                hasFreeLeft = true;
            }
        }
        if (!hasFreeLeft) cout << " (none)";
        cout << '\n';
        for (int left = 0; left < graph.leftSize; ++left) {
            cout << "      u" << left + 1 << " ->";
            bool hasOutgoing = false;
            for (int right : graph.adjacent[left]) {
                if (matchedLeft[left] != right) {
                    cout << " v" << right + 1;
                    hasOutgoing = true;
                }
            }
            if (!hasOutgoing) cout << " (none)";
            cout << '\n';
        }
        for (int right = 0; right < graph.rightSize; ++right) {
            cout << "      v" << right + 1 << " ->";
            if (matchedRight[right] == -1) cout << " t";
            else cout << " u" << matchedRight[right] + 1;
            cout << '\n';
        }
    }

public:
    MaximumMatching(const BipartiteGraph& input, bool showDetails)
        : graph(input), matchedLeft(input.leftSize, -1),
          matchedRight(input.rightSize, -1), detailed(showDetails) {}

    void solve() {
        while (true) {
            vector<int> parent;
            int finish = -1;
            if (!findAugmentingPath(parent, finish)) break;

            ++iterations;
            vector<int> path = pathWithoutTerminals(parent, finish);
            if (detailed) {
                cout << "\nIteration " << iterations << ":\n";
                cout << "  Step 3: t is reachable from s.\n";
                printPath(path);
                cout << "  Step 5: delete the first and last path arcs and reverse the intermediate arcs.\n";
            }
            augment(path);
            if (detailed) {
                printMatching();
                printResidualGraph();
            }
        }
    }

    int size() const {
        int result = 0;
        for (int right : matchedLeft) result += right != -1;
        return result;
    }

    void printResult() const {
        if (detailed)
            cout << "\nStep 4: t is not reachable from s; the algorithm stops.\n";
        cout << "\nMaximum matching:\n";
        if (size() == 0) {
            cout << "  (empty)\n";
        } else {
            for (int left = 0; left < graph.leftSize; ++left) {
                if (matchedLeft[left] != -1)
                    cout << "  (u" << left + 1 << ", v" << matchedLeft[left] + 1 << ")\n";
            }
        }
        cout << "Matching cardinality = " << size() << '\n';
        cout << "Augmentations = " << iterations << '\n';
    }
};

bool readGraph(istream& input, BipartiteGraph& graph, string& error) {
    int edgeCount;
    if (!(input >> graph.leftSize >> graph.rightSize >> edgeCount)) {
        error = "expected three integers: |V1| |V2| |E|";
        return false;
    }
    if (graph.leftSize < 0 || graph.rightSize < 0 || edgeCount < 0) {
        error = "vertex and edge counts must be non-negative";
        return false;
    }

    graph.adjacent.assign(graph.leftSize, {});
    set<pair<int, int>> uniqueEdges;
    for (int i = 0; i < edgeCount; ++i) {
        int left, right;
        if (!(input >> left >> right)) {
            error = "not enough edge pairs in the input";
            return false;
        }
        if (left < 1 || left > graph.leftSize || right < 1 || right > graph.rightSize) {
            error = "edge endpoint is outside the declared vertex sets";
            return false;
        }
        uniqueEdges.insert({left - 1, right - 1});
    }
    for (const auto& edge : uniqueEdges)
        graph.adjacent[edge.first].push_back(edge.second);
    return true;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool detailed = false;
    string filename;
    for (int i = 1; i < argc; ++i) {
        string argument = argv[i];
        if (argument == "--detailed") detailed = true;
        else if (!argument.empty() && argument[0] == '-') {
            cerr << "Unknown option: " << argument << '\n';
            cerr << "Usage: " << argv[0] << " [--detailed] [input_file]\n";
            return 2;
        }
        else if (filename.empty()) filename = argument;
        else {
            cerr << "Usage: " << argv[0] << " [--detailed] [input_file]\n";
            return 2;
        }
    }

    ifstream file;
    if (!filename.empty()) {
        file.open(filename);
        if (!file) {
            cerr << "Cannot open input file: " << filename << '\n';
            return 2;
        }
    }
    istream& input = filename.empty() ? cin : file;

    BipartiteGraph graph;
    string error;
    if (!readGraph(input, graph, error)) {
        cerr << "Input error: " << error << '\n';
        return 2;
    }
    string extra;
    if (input >> extra) {
        cerr << "Input error: unexpected data after the edge list\n";
        return 2;
    }

    cout << "Maximum Matching in a Bipartite Graph\n";
    cout << "=====================================\n";
    cout << "V1 vertices: " << graph.leftSize
         << ", V2 vertices: " << graph.rightSize << '\n';
    size_t edgeCount = 0;
    for (const auto& edges : graph.adjacent) edgeCount += edges.size();
    cout << "Edges: " << edgeCount << '\n';
    cout << "Algorithm: augmenting paths (BFS)\n";

    if (detailed) {
        cout << "\nDirected residual graph:\n";
        cout << "  s -> every free vertex of V1\n";
        cout << "  unmatched edges: V1 -> V2\n";
        cout << "  matched edges: V2 -> V1\n";
        cout << "  every free vertex of V2 -> t\n";
        cout << "\nInput edges:\n";
        for (int left = 0; left < graph.leftSize; ++left) {
            for (int right : graph.adjacent[left])
                cout << "  (u" << left + 1 << ", v" << right + 1 << ")\n";
        }
    }

    MaximumMatching solver(graph, detailed);
    solver.solve();
    solver.printResult();
    return 0;
}

/*
Input format:
  |V1| |V2| |E|
  u1 v1
  u2 v2
  ...
  uE vE

Vertices in V1 are numbered 1..|V1| and vertices in V2 are numbered 1..|V2|.
Example:
  3 3 4
  1 1
  2 1
  2 2
  3 3
*/
