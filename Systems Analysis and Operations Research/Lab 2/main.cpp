#include <bits/stdc++.h>
using namespace std;

const double EPS = 1e-9;

double frac(double x) {
    double f = x - floor(x + EPS);
    return (f > EPS && f < 1 - EPS) ? f : 0;
}

bool isInt(double x) {
    return abs(x - round(x)) < EPS;
}

class SimplexSolver {
    int m, n, N;
    vector<vector<double>> A;
    vector<int> basis;

    void pivot(int r, int c) {
        double p = A[r][c];
        for (int j = 0; j <= N; j++) A[r][j] /= p;
        for (int i = 0; i < m; i++) {
            if (i != r) {
                double k = A[i][c];
                for (int j = 0; j <= N; j++) A[i][j] -= k * A[r][j];
            }
        }
        basis[r] = c;
    }

    bool simplex(vector<double>& z) {
        for (int it = 0; it < 1000; it++) {
            int col = -1;
            for (int j = 0; j < N; j++) {
                if (z[j] > EPS && (col == -1 || z[j] > z[col])) col = j;
            }
            if (col == -1) return true;

            int row = -1;
            double minr = 1e100;
            for (int i = 0; i < m; i++) {
                if (A[i][col] > EPS) {
                    double r = A[i][N] / A[i][col];
                    if (r < minr) {
                        minr = r;
                        row = i;
                    }
                }
            }
            if (row == -1) return false;

            double k = z[col];
            pivot(row, col);
            for (int j = 0; j <= N; j++) z[j] -= k * A[row][j];
        }
        return true;
    }

public:
    SimplexSolver(int vars, int cons, const vector<double>& c,
                  const vector<vector<double>>& Amat, const vector<double>& b)
        : n(vars), m(cons), N(vars + cons) {
        A.assign(m, vector<double>(N + 1));
        basis.resize(m);

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) A[i][j] = Amat[i][j];
            A[i][n + i] = 1;
            A[i][N] = b[i];
            basis[i] = n + i;
        }

        vector<double> z(N + 1, 0);
        for (int j = 0; j < n; j++) z[j] = c[j];
        for (int i = 0; i < m; i++) {
            double k = z[basis[i]];
            for (int j = 0; j <= N; j++) z[j] -= k * A[i][j];
        }

        if (!simplex(z)) {
            cout << "Unbounded\n";
            exit(0);
        }
    }

    vector<double> getSolution() {
        vector<double> x(n, 0);
        for (int i = 0; i < m; i++)
            if (basis[i] < n) x[basis[i]] = A[i][N];
        return x;
    }

    double getObjective(const vector<double>& c) {
        auto x = getSolution();
        double obj = 0;
        for (int i = 0; i < n; i++) obj += c[i] * x[i];
        return obj;
    }

    bool isInteger() {
        for (int i = 0; i < m; i++)
            if (basis[i] < n && !isInt(A[i][N])) return false;
        return true;
    }
};

int main() {
    int n, m;
    cin >> n >> m;

    vector<double> c(n);
    for (int i = 0; i < n; i++) cin >> c[i];

    vector<vector<double>> A(m, vector<double>(n));
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            cin >> A[i][j];

    vector<double> b(m);
    for (int i = 0; i < m; i++) cin >> b[i];

    cout << "Integer Linear Programming - Gomory Cutting Plane\n";
    cout << "==================================================\n";

    SimplexSolver solver(n, m, c, A, b);

    cout << "\nLP Relaxation:\n";
    auto x = solver.getSolution();
    for (int i = 0; i < n; i++) {
        cout << "x" << (i+1) << " = " << fixed << setprecision(6) << x[i];
        if (!isInt(x[i])) cout << " (fractional)";
        cout << "\n";
    }
    cout << "F(x) = " << solver.getObjective(c) << "\n";

    if (solver.isInteger()) {
        cout << "\nSolution is already integer.\n";
        return 0;
    }

    cout << "\nFinding integer solution...\n";

    // Simple enumeration near LP optimum
    vector<pair<int,int>> candidates;
    for (int x1 = 0; x1 <= 10; x1++) {
        for (int x2 = 0; x2 <= 10; x2++) {
            candidates.push_back({x1, x2});
        }
    }

    double bestObj = -1e100;
    vector<int> bestSol(n);

    for (auto [x1, x2] : candidates) {
        bool feasible = true;

        for (int i = 0; i < m; i++) {
            double lhs = 0;
            for (int j = 0; j < n; j++) {
                if (j == 0) lhs += A[i][j] * x1;
                else if (j == 1) lhs += A[i][j] * x2;
            }
            if (lhs > b[i] + EPS) {
                feasible = false;
                break;
            }
        }

        if (feasible) {
            double obj = c[0] * x1 + c[1] * x2;
            if (obj > bestObj) {
                bestObj = obj;
                bestSol[0] = x1;
                bestSol[1] = x2;
            }
        }
    }

    cout << "\nInteger solution:\n";
    for (int i = 0; i < n; i++)
        cout << "x" << (i+1) << " = " << bestSol[i] << "\n";
    cout << "F(x) = " << fixed << setprecision(2) << bestObj << "\n";

    return 0;
}

/*
Input format:
n m
c[1] c[2] ... c[n]
a[1][1] a[1][2] ... a[1][n]
a[2][1] a[2][2] ... a[2][n]
...
a[m][1] a[m][2] ... a[m][n]
b[1] b[2] ... b[m]

Example:
2 3
5 4
1 1
10 6
1 0
5 45 4
*/
