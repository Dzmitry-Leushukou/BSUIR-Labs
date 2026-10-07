#include <bits/stdc++.h>
using namespace std;

const double EPS = 1e-9;

// Simplex method for: max c*x, A*x <= b, x >= 0.
class Simplex {
    int m, n;
    vector<vector<double>> D;
    vector<int> B, N;
    bool verbose;

    void printTable(const string& title) {
        if (!verbose) return;

        cout << "\n" << title << "\n";
        cout << string(80, '-') << "\n";

        // Header
        cout << setw(8) << "Basis" << " | ";
        for (int j = 0; j <= n; ++j) {
            if (N[j] == -1) cout << setw(10) << "s";
            else cout << setw(10) << ("x" + to_string(N[j] + 1));
        }
        cout << " | " << setw(10) << "RHS" << "\n";
        cout << string(80, '-') << "\n";

        // Basis rows
        for (int i = 0; i < m; ++i) {
            if (B[i] == -1) cout << setw(8) << "s";
            else if (B[i] >= n) cout << setw(8) << ("s" + to_string(B[i] - n + 1));
            else cout << setw(8) << ("x" + to_string(B[i] + 1));
            cout << " | ";
            for (int j = 0; j <= n; ++j) {
                cout << setw(10) << fixed << setprecision(4) << D[i][j];
            }
            cout << " | " << setw(10) << fixed << setprecision(4) << D[i][n + 1] << "\n";
        }

        cout << string(80, '-') << "\n";

        // Objective row (Phase 2)
        cout << setw(8) << "z" << " | ";
        for (int j = 0; j <= n; ++j) {
            cout << setw(10) << fixed << setprecision(4) << D[m][j];
        }
        cout << " | " << setw(10) << fixed << setprecision(4) << D[m][n + 1] << "\n";

        // Auxiliary objective row (Phase 1)
        if (D[m + 1][n] != 0) {
            cout << setw(8) << "w" << " | ";
            for (int j = 0; j <= n; ++j) {
                cout << setw(10) << fixed << setprecision(4) << D[m + 1][j];
            }
            cout << " | " << setw(10) << fixed << setprecision(4) << D[m + 1][n + 1] << "\n";
        }

        cout << string(80, '-') << "\n";
    }

    void pivot(int r, int s) {
        if (verbose) {
            string enterVar = (N[s] == -1) ? "s" : ("x" + to_string(N[s] + 1));
            string leaveVar = (B[r] == -1) ? "s" :
                              (B[r] >= n) ? ("s" + to_string(B[r] - n + 1)) :
                              ("x" + to_string(B[r] + 1));
            cout << "\n>>> Pivot: " << enterVar << " enters, " << leaveVar
                 << " leaves (pivot element = " << fixed << setprecision(4)
                 << D[r][s] << ")\n";
        }

        double inv = 1.0 / D[r][s];
        for (int i = 0; i < m + 2; ++i)
            if (i != r)
                for (int j = 0; j < n + 2; ++j)
                    if (j != s)
                        D[i][j] -= D[r][j] * D[i][s] * inv;
        for (int j = 0; j < n + 2; ++j) if (j != s) D[r][j] *= inv;
        for (int i = 0; i < m + 2; ++i) if (i != r) D[i][s] *= -inv;
        D[r][s] = inv;
        swap(B[r], N[s]);
    }

    bool phase(int ph) {
        int x = ph == 1 ? m + 1 : m;

        if (verbose) {
            if (ph == 1) {
                cout << "\n========== PHASE 1: Finding initial feasible solution ==========\n";
            } else {
                cout << "\n========== PHASE 2: Optimizing objective function ==========\n";
            }
            printTable("Initial tableau for Phase " + to_string(ph));
        }

        while (true) {
            int s = -1;
            for (int j = 0; j <= n; ++j) {
                if (ph == 2 && N[j] == -1) continue;
                if (s == -1 || D[x][j] < D[x][s] - EPS ||
                    (abs(D[x][j] - D[x][s]) <= EPS && N[j] < N[s])) s = j;
            }
            if (D[x][s] >= -EPS) {
                if (verbose) {
                    cout << "\nPhase " << ph << " complete: all reduced costs >= 0\n";
                    printTable("Final tableau for Phase " + to_string(ph));
                }
                return true;
            }

            int r = -1;
            for (int i = 0; i < m; ++i) if (D[i][s] > EPS) {
                if (r == -1) r = i;
                else {
                    double a = D[i][n + 1] / D[i][s];
                    double b = D[r][n + 1] / D[r][s];
                    if (a < b - EPS || (abs(a - b) <= EPS && B[i] < B[r])) r = i;
                }
            }
            if (r == -1) {
                if (verbose) cout << "\nUnbounded: no leaving variable found\n";
                return false;
            }

            pivot(r, s);

            if (verbose) {
                printTable("After pivot");
            }
        }
    }

public:
    Simplex(const vector<vector<double>>& A, const vector<double>& b,
            const vector<double>& c, bool verb = false)
        : m((int)b.size()), n((int)c.size()),
          D(m + 2, vector<double>(n + 2)), B(m), N(n + 1), verbose(verb) {
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j) D[i][j] = A[i][j];
        for (int i = 0; i < m; ++i) {
            B[i] = n + i;
            D[i][n] = -1;
            D[i][n + 1] = b[i];
        }
        for (int j = 0; j < n; ++j) {
            N[j] = j;
            D[m][j] = -c[j];
        }
        N[n] = -1;
        D[m + 1][n] = 1;
    }

    bool solve(vector<double>& x, double& value, bool verboseSimplex) {
        verbose = verboseSimplex;
        int r = 0;
        for (int i = 1; i < m; ++i)
            if (D[i][n + 1] < D[r][n + 1]) r = i;

        if (D[r][n + 1] < -EPS) {
            if (verbose) {
                cout << "\nInitial basis infeasible (negative RHS detected)\n";
                cout << "Adding artificial variable and starting Phase 1\n";
            }
            pivot(r, n);
            if (!phase(1) || D[m + 1][n + 1] < -EPS) return false;
            if (abs(D[m + 1][n + 1]) > EPS) return false;
            auto it = find(B.begin(), B.end(), -1);
            if (it != B.end()) {
                r = int(it - B.begin());
                int s = -1;
                for (int j = 0; j <= n; ++j)
                    if (abs(D[r][j]) > EPS) { s = j; break; }
                if (s != -1) pivot(r, s);
            }
        } else {
            if (verbose) {
                cout << "\nInitial basis is feasible, skipping Phase 1\n";
            }
        }

        while (true) {
            if (phase(2)) break;
            return false;
        }

        x.assign(n, 0);
        for (int i = 0; i < m; ++i)
            if (B[i] < n) x[B[i]] = D[i][n + 1];
        value = D[m][n + 1];

        if (verbose) {
            cout << "\nOptimal solution found:\n";
            cout << "  x = (";
            for (int j = 0; j < n; ++j) {
                if (j) cout << ", ";
                cout << fixed << setprecision(4) << x[j];
            }
            cout << ")\n";
            cout << "  Objective value = " << fixed << setprecision(4) << value << "\n";
        }

        return true;
    }
};

struct Problem {
    int n, m;
    vector<double> c, b, lo, hi;
    vector<vector<double>> A;
};

struct Node {
    vector<double> lo, hi;
    int id;
};

class BranchAndBound {
    const Problem& p;
    bool verboseNodes;
    bool verboseSimplex;
    int nextId = 1;
    int nodes = 0;
    bool found = false;
    double bestValue = -1e100;
    vector<double> bestX;

    static bool isInteger(double x) {
        return abs(x - round(x)) <= 1e-7;
    }

    bool solveRelaxation(const Node& node, vector<double>& x, double& value) {
        const int n = p.n;
        vector<vector<double>> A = p.A;
        vector<double> b = p.b;

        // Shift x = lo + y, y >= 0.
        // A*y <= b - A*lo.
        for (int i = 0; i < p.m; ++i) {
            for (int j = 0; j < n; ++j)
                b[i] -= p.A[i][j] * node.lo[j];
        }

        // y_j <= hi_j - lo_j.
        for (int j = 0; j < n; ++j) {
            if (node.lo[j] > node.hi[j] + EPS) return false;
            vector<double> row(n, 0);
            row[j] = 1;
            A.push_back(row);
            b.push_back(node.hi[j] - node.lo[j]);
        }

        // Check feasibility
        for (double rhs : b)
            if (rhs < -EPS) return false;

        vector<double> y;
        double shiftedValue;
        Simplex simplex(A, b, p.c, verboseSimplex);

        if (verboseSimplex) {
            cout << "\n  Solving LP relaxation for this node:\n";
            cout << "  Shifted bounds: ";
            for (int j = 0; j < n; ++j) {
                cout << "0 <= y" << j + 1 << " <= " << fixed << setprecision(4)
                     << (node.hi[j] - node.lo[j]);
                if (j + 1 < n) cout << ", ";
            }
            cout << "\n";
        }

        if (!simplex.solve(y, shiftedValue, verboseSimplex)) return false;

        x.resize(n);
        value = shiftedValue;
        for (int j = 0; j < n; ++j) {
            x[j] = node.lo[j] + y[j];
            value += p.c[j] * node.lo[j];
        }
        return true;
    }

    void printVector(const vector<double>& x) const {
        cout << fixed << setprecision(4);
        cout << "    x = (";
        for (int i = 0; i < (int)x.size(); ++i) {
            if (i) cout << ", ";
            cout << x[i];
        }
        cout << ")\n";
    }

public:
    BranchAndBound(const Problem& problem, bool showNodes, bool showSimplex = false)
        : p(problem), verboseNodes(showNodes), verboseSimplex(showSimplex) {}

    bool solve() {
        // Step 2: Initialize x*, r, and empty stack S
        stack<Node> S;
        S.push({p.lo, p.hi, 1});

        if (verboseNodes) {
            cout << "\n========== Step 2: Initialize Stack ==========\n";
            cout << "Starting Branch and Bound with explicit stack\n";
        }

        // Step 4: Main loop
        while (!S.empty()) {
            // Case 2: Stack is non-empty - extract task from stack
            Node node = S.top();
            S.pop();
            ++nodes;

            if (verboseNodes) {
                cout << "\n[Node " << node.id << "]" << " (Stack size: " << S.size() << ")\n";
                cout << "    Bounds: ";
                for (int i = 0; i < p.n; ++i) {
                    cout << node.lo[i] << " <= x" << i + 1
                         << " <= " << node.hi[i];
                    if (i + 1 < p.n) cout << ", ";
                }
                cout << '\n';
            }

            vector<double> x;
            double bound;

            // Solve LP relaxation
            if (!solveRelaxation(node, x, bound)) {
                if (verboseNodes) cout << "    Pruned: LP relaxation is infeasible.\n";
                continue;
            }

            if (verboseNodes) {
                printVector(x);
                cout << "    LP bound = " << bound << '\n';
            }

            // Pruning by bound
            if (found && bound <= bestValue + EPS) {
                if (verboseNodes) cout << "    Pruned: bound <= incumbent (" << bestValue << ").\n";                continue;
            }

            // Check integrality
            int branch = -1;
            for (int j = 0; j < p.n; ++j) {
                if (!isInteger(x[j])) {
                    branch = j;
                    break;
                }
            }

            if (branch == -1) {
                // Integer solution found
                found = true;
                bestValue = bound;
                bestX = x;
                if (verboseNodes) cout << "    Integer solution found! F(x) = " << bestValue << "\n";
                continue;
            }

            // Branching
            double floorValue = floor(x[branch]);
            double ceilValue = ceil(x[branch]);

            if (verboseNodes) {
                cout << "    Branch on x" << branch + 1 << " = " << x[branch] << '\n';
                cout << "    Left : x" << branch + 1 << " <= " << floorValue << '\n';
                cout << "    Right: x" << branch + 1 << " >= " << ceilValue << '\n';
            }

            // Right child: x[branch] >= ceil(x[branch])
            if (ceilValue <= node.hi[branch] + EPS) {
                Node right = node;
                right.id = ++nextId;
                right.lo[branch] = max(right.lo[branch], ceilValue);
                S.push(right);
            }

            // Left child: x[branch] <= floor(x[branch])
            if (floorValue >= node.lo[branch] - EPS) {
                Node left = node;
                left.id = ++nextId;
                left.hi[branch] = min(left.hi[branch], floorValue);
                S.push(left);
            }
        }

        // Case 1: Stack is empty - algorithm terminates
        return found;
    }

    void printResult() const {
        cout << "\n========== RESULT ==========\n";
        if (!found) {
            cout << "No integer solution found.\n";
            return;
        }

        cout << fixed << setprecision(4);

        for (int i = 0; i < p.n; ++i)
            cout << "x" << i + 1 << " = " << bestX[i] << '\n';
        cout << "F(x) = " << bestValue << '\n';
        cout << "Nodes processed: " << nodes << '\n';
    }
};

int main(int argc, char* argv[]) {
    bool detailedOutput = false;
    string filename;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--detailed") {
            detailedOutput = true;
        } else {
            filename = arg;
        }
    }

    if (!filename.empty()) {
        freopen(filename.c_str(), "r", stdin);
    }

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Problem p;
    cin >> p.n >> p.m;

    p.c.resize(p.n);
    for (double& v : p.c) cin >> v;

    p.A.assign(p.m, vector<double>(p.n));
    for (auto& row : p.A)
        for (double& v : row) cin >> v;

    p.b.resize(p.m);
    for (double& v : p.b) cin >> v;

    p.lo.resize(p.n);
    for (double& v : p.lo) cin >> v;

    p.hi.resize(p.n);
    for (double& v : p.hi) cin >> v;

    cout << "Branch and Bound - Integer Linear Programming\n";
    cout << "==============================================\n";
    cout << "Objective: maximize F(x)\n";
    cout << "Variables: " << p.n << ", constraints: " << p.m << "\n";
    if (detailedOutput) {
        cout << "Output mode: DETAILED (showing all simplex iterations)\n";
    } else {
        cout << "Output mode: BRIEF (use --detailed flag for full simplex tables)\n";
    }

    BranchAndBound solver(p, true, detailedOutput);
    solver.solve();
    solver.printResult();
}


/*
2 2
1 1
5 9
9 5
63 63
1 1
6 6
*/
