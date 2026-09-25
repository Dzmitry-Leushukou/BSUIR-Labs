#include <bits/stdc++.h>
using namespace std;

const double EPS = 1e-9;

// Simplex method for: max c*x, A*x <= b, x >= 0.
class Simplex {
    int m, n;
    vector<vector<double>> D;
    vector<int> B, N;

    void pivot(int r, int s) {
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
        while (true) {
            int s = -1;
            for (int j = 0; j <= n; ++j) {
                if (ph == 2 && N[j] == -1) continue;
                if (s == -1 || D[x][j] < D[x][s] - EPS ||
                    (abs(D[x][j] - D[x][s]) <= EPS && N[j] < N[s])) s = j;
            }
            if (D[x][s] >= -EPS) return true;

            int r = -1;
            for (int i = 0; i < m; ++i) if (D[i][s] > EPS) {
                if (r == -1) r = i;
                else {
                    double a = D[i][n + 1] / D[i][s];
                    double b = D[r][n + 1] / D[r][s];
                    if (a < b - EPS || (abs(a - b) <= EPS && B[i] < B[r])) r = i;
                }
            }
            if (r == -1) return false;
            pivot(r, s);
        }
    }

public:
    Simplex(const vector<vector<double>>& A, const vector<double>& b,
            const vector<double>& c)
        : m((int)b.size()), n((int)c.size()),
          D(m + 2, vector<double>(n + 2)), B(m), N(n + 1) {
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

    bool solve(vector<double>& x, double& value, bool verbose) {
        int r = 0;
        for (int i = 1; i < m; ++i)
            if (D[i][n + 1] < D[r][n + 1]) r = i;

        int step = 0;
        auto doPivot = [&](int rr, int ss) {
            if (verbose)
                cout << "    Simplex step " << ++step
                     << ": enter variable " << ss + 1
                     << ", leave row " << rr + 1 << '\n';
            pivot(rr, ss);
        };

        if (D[r][n + 1] < -EPS) {
            doPivot(r, n);
            if (!phase(1) || D[m + 1][n + 1] < -EPS) return false;
            if (abs(D[m + 1][n + 1]) > EPS) return false;
            auto it = find(B.begin(), B.end(), -1);
            if (it != B.end()) {
                r = int(it - B.begin());
                int s = -1;
                for (int j = 0; j <= n; ++j)
                    if (abs(D[r][j]) > EPS) { s = j; break; }
                if (s != -1) doPivot(r, s);
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
    bool verbose;
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

        // A feasible node must have non-negative right-hand sides.
        for (double rhs : b)
            if (rhs < -EPS) return false;

        vector<double> y;
        double shiftedValue;
        Simplex simplex(A, b, p.c);

        if (!simplex.solve(y, shiftedValue, verbose)) return false;

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

    void dfs(const Node& node) {
        ++nodes;
        if (verbose) {
            cout << "\n[Node " << node.id << "]\n";
            cout << "    Bounds: ";
            for (int i = 0; i < p.n; ++i)
                cout << node.lo[i] << " <= x" << i + 1
                     << " <= " << node.hi[i] << (i + 1 == p.n ? '\n' : ',');
        }

        vector<double> x;
        double bound;

        if (!solveRelaxation(node, x, bound)) {
            if (verbose) cout << "    Pruned: LP relaxation is infeasible.\n";
            return;
        }

        if (verbose) {
            printVector(x);
            cout << "    LP bound = " << bound << '\n';
        }

        if (found && bound <= bestValue + EPS) {
            if (verbose) cout << "    Pruned: bound is not better than incumbent.\n";
            return;
        }

        int branch = -1;
        for (int j = 0; j < p.n; ++j) {
            if (!isInteger(x[j])) {
                branch = j;
                break;
            }
        }

        if (branch == -1) {
            found = true;
            bestValue = bound;
            bestX = x;
            if (verbose) cout << "    Integer solution found!\n";
            return;
        }

        double floorValue = floor(x[branch]);
        double ceilValue = ceil(x[branch]);

        if (verbose) {
            cout << "    Branch on x" << branch + 1
                 << " = " << x[branch] << '\n';
            cout << "    Left : x" << branch + 1 << " <= " << floorValue << '\n';
            cout << "    Right: x" << branch + 1 << " >= " << ceilValue << '\n';
        }

        if (floorValue >= node.lo[branch] - EPS) {
            Node left = node;
            left.id = ++nextId;
            left.hi[branch] = min(left.hi[branch], floorValue);
            dfs(left);
        }

        if (ceilValue <= node.hi[branch] + EPS) {
            Node right = node;
            right.id = ++nextId;
            right.lo[branch] = max(right.lo[branch], ceilValue);
            dfs(right);
        }
    }

public:
    BranchAndBound(const Problem& problem, bool showSteps)
        : p(problem), verbose(showSteps) {}

    bool solve() {
        dfs({p.lo, p.hi, 1});
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

int main() {
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

    BranchAndBound solver(p, true);
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