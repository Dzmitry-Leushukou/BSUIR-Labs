#include <algorithm>
#include <cstdio>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

using namespace std;

class FordFulkerson {
    int vertexCount;
    int source;
    int sink;
    vector<vector<long long>> capacity;
    vector<vector<long long>> flow;
    vector<vector<bool>> hasArc;
    long long maxFlow = 0;
    int iteration = 0;

    long long residualCapacity(int from, int to) const {
        return capacity[from][to] - flow[from][to] + flow[to][from];
    }

    bool findPath(vector<int>& parent) const {
        fill(parent.begin(), parent.end(), -1);
        queue<int> pending;
        pending.push(source);
        parent[source] = source;

        while (!pending.empty() && parent[sink] == -1) {
            int vertex = pending.front();
            pending.pop();
            for (int next = 0; next < vertexCount; ++next) {
                if (parent[next] != -1 || residualCapacity(vertex, next) <= 0) continue;
                parent[next] = vertex;
                pending.push(next);
            }
        }
        return parent[sink] != -1;
    }

public:
    FordFulkerson(int count, int start, int finish)
        : vertexCount(count), source(start), sink(finish),
          capacity(count, vector<long long>(count, 0)),
          flow(count, vector<long long>(count, 0)),
          hasArc(count, vector<bool>(count, false)) {}

    bool addArc(int from, int to, long long arcCapacity) {
        if (hasArc[from][to]) return false;
        hasArc[from][to] = true;
        capacity[from][to] = arcCapacity;
        return true;
    }

    bool hasAllReverseArcs() const {
        for (int from = 0; from < vertexCount; ++from)
            for (int to = 0; to < vertexCount; ++to)
                if (hasArc[from][to] && !hasArc[to][from]) return false;
        return true;
    }

    void solve() {
        vector<int> parent(vertexCount);
        while (findPath(parent)) {
            vector<int> path;
            long long augmentation = numeric_limits<long long>::max();
            for (int vertex = sink; vertex != source; vertex = parent[vertex]) {
                augmentation = min(augmentation, residualCapacity(parent[vertex], vertex));
                path.push_back(vertex);
            }
            path.push_back(source);
            reverse(path.begin(), path.end());

            ++iteration;
            cout << "\nAugmenting path " << iteration << ": ";
            for (size_t i = 0; i < path.size(); ++i) {
                if (i) cout << " -> ";
                cout << path[i] + 1;
            }
            cout << "\nBottleneck capacity: " << augmentation << '\n';

            vector<vector<long long>> pathFlow(vertexCount, vector<long long>(vertexCount, 0));
            for (size_t i = 1; i < path.size(); ++i)
                pathFlow[path[i - 1]][path[i]] = augmentation;

            vector<vector<long long>> updatedFlow = flow;
            for (int from = 0; from < vertexCount; ++from) {
                for (int to = 0; to < vertexCount; ++to) {
                    updatedFlow[from][to] = max(
                        0LL,
                        flow[from][to] - flow[to][from] +
                            pathFlow[from][to] - pathFlow[to][from]);
                }
            }
            flow.swap(updatedFlow);
            maxFlow += augmentation;
        }
    }

    void printResult() const {
        cout << "\nMaximum flow: " << maxFlow << '\n';
        cout << "\nFlow on arcs (flow/capacity):\n";
        for (int from = 0; from < vertexCount; ++from)
            for (int to = 0; to < vertexCount; ++to)
                if (hasArc[from][to])
                    cout << from + 1 << " -> " << to + 1 << ": "
                         << flow[from][to] << '/' << capacity[from][to] << '\n';

        vector<bool> reachable(vertexCount, false);
        queue<int> pending;
        pending.push(source);
        reachable[source] = true;
        while (!pending.empty()) {
            int vertex = pending.front();
            pending.pop();
            for (int next = 0; next < vertexCount; ++next) {
                if (!reachable[next] && residualCapacity(vertex, next) > 0) {
                    reachable[next] = true;
                    pending.push(next);
                }
            }
        }

        cout << "\nMinimum cut: S = {";
        bool first = true;
        for (int i = 0; i < vertexCount; ++i) {
            if (!reachable[i]) continue;
            if (!first) cout << ", ";
            cout << i + 1;
            first = false;
        }
        cout << "}, T = {";
        first = true;
        for (int i = 0; i < vertexCount; ++i) {
            if (reachable[i]) continue;
            if (!first) cout << ", ";
            cout << i + 1;
            first = false;
        }
        cout << "}\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc > 2) {
        cerr << "Usage: lab8 [input_file]\n";
        return 1;
    }
    if (argc == 2 && !freopen(argv[1], "r", stdin)) {
        cerr << "Error: cannot open input file '" << argv[1] << "'.\n";
        return 1;
    }

    int vertexCount, arcCount, source, sink;
    if (!(cin >> vertexCount >> arcCount >> source >> sink)) {
        cerr << "Input must start with: vertex_count arc_count source sink.\n";
        return 1;
    }
    if (vertexCount < 2 || arcCount < 0 || source < 1 || source > vertexCount ||
        sink < 1 || sink > vertexCount || source == sink) {
        cerr << "Error: invalid graph size or source/sink.\n";
        return 1;
    }

    FordFulkerson solver(vertexCount, source - 1, sink - 1);
    for (int i = 0; i < arcCount; ++i) {
        int from, to;
        long long arcCapacity;
        if (!(cin >> from >> to >> arcCapacity)) {
            cerr << "Error: missing data for arc " << i + 1 << ".\n";
            return 1;
        }
        if (from < 1 || from > vertexCount || to < 1 || to > vertexCount ||
            from == to || arcCapacity < 0) {
            cerr << "Error: invalid arc " << i + 1 << ".\n";
            return 1;
        }
        if (!solver.addArc(from - 1, to - 1, arcCapacity)) {
            cerr << "Error: duplicate arc " << from << " -> " << to << ".\n";
            return 1;
        }
    }

    string extra;
    if (cin >> extra) {
        cerr << "Error: unexpected data after the last arc.\n";
        return 1;
    }
    if (!solver.hasAllReverseArcs()) {
        cerr << "Error: every arc must have its reverse arc in the input, "
                "including reverse arcs with zero capacity.\n";
        return 1;
    }

    cout << "Ford-Fulkerson algorithm (labeling method)\n";
    cout << "===========================================\n";
    cout << "Source: " << source << ", sink: " << sink << '\n';
    solver.solve();
    solver.printResult();
    return 0;
}
