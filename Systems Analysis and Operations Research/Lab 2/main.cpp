#include <bits/stdc++.h>
using namespace std;

const double EPS = 1e-9;
const int MAX_GOMORY_CUTS = 1000;

double fractionalPart(double value) {
    double result = value - floor(value);
    return abs(result) <= EPS || abs(1.0 - result) <= EPS ? 0.0 : result;
}

bool isInteger(double value) {
    return abs(value - round(value)) <= 1e-7;
}

enum class SolveStatus { Optimal, Infeasible, Unbounded };

struct LPSolution {
    SolveStatus status = SolveStatus::Infeasible;
    vector<double> x;
    vector<int> basis;
    vector<vector<double>> rows;
    vector<double> rhs;
    vector<int> nonbasic;
    double value = 0;
};

long long gcdll(long long a, long long b) {
    return b == 0 ? llabs(a) : gcdll(b, a % b);
}

long long approximateDenominator(double value) {
    if (abs(value) <= EPS) return 1;
    const long long maxDenominator = 10000;
    long long bestDenominator = 1;
    double bestError = abs(value - round(value));
    for (long long denominator = 2; denominator <= maxDenominator; ++denominator) {
        double error = abs(value - round(value * denominator) / denominator);
        if (error < bestError) {
            bestError = error;
            bestDenominator = denominator;
            if (error <= 1e-10) break;
        }
    }
    if (bestError > 1e-7)
        throw runtime_error("Cannot represent a Gomory cut coefficient reliably as a rational number.");
    return bestDenominator;
}

vector<long long> integerizeCut(const vector<double>& fractionalCoefficients, double fractionalRhs) {
    vector<double> values = fractionalCoefficients;
    values.push_back(fractionalRhs);
    long long scale = 1;
    for (double value : values) {
        long long denominator = approximateDenominator(value);
        long long divisor = gcdll(scale, denominator);
        if (scale > 1000000000LL / (denominator / divisor))
            throw runtime_error("Gomory cut denominator is too large to scale safely.");
        scale *= denominator / divisor;
    }

    vector<long long> result;
    result.reserve(values.size());
    for (double value : values)
        result.push_back(llround(value * scale));
    long long commonDivisor = 0;
    for (long long value : result) commonDivisor = gcdll(commonDivisor, value);
    if (commonDivisor > 1)
        for (long long& value : result) value /= commonDivisor;
    return result;
}

class Simplex {
    int rowsCount;
    int variableCount;
    vector<vector<double>> tableau;
    vector<double> rhs;
    vector<int> basis;

    bool optimize(const vector<double>& costs, int enteringLimit, double& value,
                  const string& phaseName, bool detailed) {
        vector<double> reduced(variableCount, 0.0);
        value = 0.0;
        for (int j = 0; j < variableCount; ++j) reduced[j] = costs[j];
        for (int i = 0; i < rowsCount; ++i) {
            double basicCost = costs[basis[i]];
            value += basicCost * rhs[i];
            if (abs(basicCost) <= EPS) continue;
            for (int j = 0; j < variableCount; ++j)
                reduced[j] -= basicCost * tableau[i][j];
        }

        if (detailed) {
            cout << "\n========== " << phaseName << " ==========\n";
            printTableau();
        }
        for (int iteration = 0; iteration < 10000; ++iteration) {
            int entering = -1;
            for (int j = 0; j < enteringLimit; ++j) {
                if (reduced[j] > EPS &&
                    (entering == -1 || reduced[j] > reduced[entering] + EPS ||
                     (abs(reduced[j] - reduced[entering]) <= EPS && j < entering))) {
                    entering = j;
                }
            }
            if (entering == -1) {
                if (detailed) {
                    cout << "No improving variable remains; this phase is optimal.\n";
                    printTableau();
                }
                return true;
            }

            int leaving = -1;
            double bestRatio = numeric_limits<double>::infinity();
            for (int i = 0; i < rowsCount; ++i) {
                if (tableau[i][entering] <= EPS) continue;
                double ratio = rhs[i] / tableau[i][entering];
                if (ratio < bestRatio - EPS ||
                    (abs(ratio - bestRatio) <= EPS &&
                     (leaving == -1 || basis[i] < basis[leaving]))) {
                    bestRatio = ratio;
                    leaving = i;
                }
            }
            if (leaving == -1) {
                if (detailed) cout << "Unbounded: no leaving variable exists.\n";
                return false;
            }

            double pivot = tableau[leaving][entering];
            int leavingVariable = basis[leaving];
            double oldRhs = rhs[leaving];
            for (int j = 0; j < variableCount; ++j)
                tableau[leaving][j] /= pivot;
            rhs[leaving] /= pivot;

            for (int i = 0; i < rowsCount; ++i) {
                if (i == leaving) continue;
                double factor = tableau[i][entering];
                if (abs(factor) <= EPS) continue;
                for (int j = 0; j < variableCount; ++j)
                    tableau[i][j] -= factor * tableau[leaving][j];
                rhs[i] -= factor * rhs[leaving];
                if (abs(rhs[i]) <= EPS) rhs[i] = 0;
            }

            double factor = reduced[entering];
            for (int j = 0; j < variableCount; ++j)
                reduced[j] -= factor * tableau[leaving][j];
            value += factor * rhs[leaving];
            basis[leaving] = entering;

            if (detailed) {
                cout << "Pivot " << iteration + 1 << ": x" << entering + 1
                     << " enters, x" << leavingVariable + 1 << " leaves; "
                     << "pivot element = " << fixed << setprecision(6) << pivot
                     << ", previous RHS = " << oldRhs << "\n";
                printTableau();
            }
        }
        throw runtime_error("Simplex iteration limit exceeded.");
    }

    void printTableau() const {
        cout << "\nCurrent basis tableau:\n";
        cout << setw(10) << "Basis" << " | ";
        for (int j = 0; j < variableCount; ++j)
            cout << setw(10) << ("x" + to_string(j + 1));
        cout << " | " << setw(10) << "RHS" << '\n';
        cout << string(12 + (variableCount + 1) * 11, '-') << '\n';
        for (int i = 0; i < rowsCount; ++i) {
            cout << setw(10) << ("x" + to_string(basis[i] + 1)) << " | ";
            for (int j = 0; j < variableCount; ++j)
                cout << setw(10) << fixed << setprecision(4) << tableau[i][j];
            cout << " | " << setw(10) << fixed << setprecision(4) << rhs[i] << '\n';
        }
    }

public:
    Simplex(const vector<vector<double>>& coefficients,
            const vector<double>& rightHandSide, int originalVariables)
        : rowsCount((int)rightHandSide.size()),
          variableCount(originalVariables + (int)rightHandSide.size()),
          tableau(rowsCount, vector<double>(variableCount, 0.0)),
          rhs(rightHandSide), basis(rowsCount) {
        for (int i = 0; i < rowsCount; ++i) {
            double sign = rhs[i] < 0 ? -1.0 : 1.0;
            rhs[i] *= sign;
            for (int j = 0; j < originalVariables; ++j)
                tableau[i][j] = sign * coefficients[i][j];
            int artificial = originalVariables + i;
            tableau[i][artificial] = 1.0;
            basis[i] = artificial;
        }
    }

    LPSolution solve(const vector<double>& objective, bool detailed) {
        vector<double> phaseOneCosts(variableCount, 0.0);
        for (int j = 0; j < variableCount; ++j)
            if (j >= (int)objective.size()) phaseOneCosts[j] = -1.0;

        double phaseOneValue = 0;
        if (!optimize(phaseOneCosts, variableCount, phaseOneValue,
                      "PHASE 1: Find a feasible basis", detailed))
            throw runtime_error("Phase 1 unexpectedly became unbounded.");
        if (phaseOneValue < -1e-7) {
            LPSolution result;
            result.status = SolveStatus::Infeasible;
            return result;
        }

        int originalVariables = (int)objective.size();
        for (int i = 0; i < rowsCount;) {
            if (basis[i] < originalVariables) {
                ++i;
                continue;
            }
            int entering = -1;
            for (int j = 0; j < originalVariables; ++j) {
                if (abs(tableau[i][j]) > EPS) {
                    entering = j;
                    break;
                }
            }
            if (entering == -1) {
                if (abs(rhs[i]) > 1e-7) {
                    LPSolution result;
                    result.status = SolveStatus::Infeasible;
                    return result;
                }
                tableau.erase(tableau.begin() + i);
                rhs.erase(rhs.begin() + i);
                basis.erase(basis.begin() + i);
                --rowsCount;
                continue;
            }
            pivot(i, entering);
            ++i;
        }

        double optimum = 0;
        vector<double> phaseTwoCosts(variableCount, 0.0);
        copy(objective.begin(), objective.end(), phaseTwoCosts.begin());
        if (!optimize(phaseTwoCosts, originalVariables, optimum,
                      "PHASE 2: Optimize the objective", detailed)) {
            LPSolution result;
            result.status = SolveStatus::Unbounded;
            return result;
        }

        LPSolution result;
        result.status = SolveStatus::Optimal;
        result.x.assign(originalVariables, 0.0);
        result.basis = basis;
        result.rows = tableau;
        result.rhs = rhs;
        result.value = optimum;
        for (int i = 0; i < rowsCount; ++i)
            if (basis[i] < originalVariables) result.x[basis[i]] = rhs[i];
        vector<bool> isBasic(originalVariables, false);
        for (int index : basis)
            if (index < originalVariables) isBasic[index] = true;
        for (int j = 0; j < originalVariables; ++j)
            if (!isBasic[j]) result.nonbasic.push_back(j);
        return result;
    }

private:
    void pivot(int row, int column) {
        double value = tableau[row][column];
        for (int j = 0; j < variableCount; ++j)
            tableau[row][j] /= value;
        rhs[row] /= value;
        for (int i = 0; i < rowsCount; ++i) {
            if (i == row) continue;
            double factor = tableau[i][column];
            if (abs(factor) <= EPS) continue;
            for (int j = 0; j < variableCount; ++j)
                tableau[i][j] -= factor * tableau[row][j];
            rhs[i] -= factor * rhs[row];
        }
        basis[row] = column;
    }
};

void printVector(const vector<double>& x) {
    cout << '(';
    for (int i = 0; i < (int)x.size(); ++i) {
        if (i) cout << ", ";
        cout << "x" << i + 1 << " = " << fixed << setprecision(6) << x[i];
    }
    cout << ')';
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool detailed = true;
    string filename;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--detailed") detailed = true;
        else if (arg == "--brief") detailed = false;
        else if (filename.empty()) filename = arg;
        else {
            cerr << "Usage: " << argv[0] << " [--detailed|--brief] [input_file]\n";
            return 2;
        }
    }
    ifstream inputFile;
    if (!filename.empty()) {
        inputFile.open(filename);
        if (!inputFile) {
            cerr << "Cannot open input file: " << filename << '\n';
            return 2;
        }
        cin.rdbuf(inputFile.rdbuf());
    }

    int n, m;
    if (!(cin >> n >> m) || n <= 0 || m <= 0) {
        cerr << "Input error: expected positive variable and constraint counts (n m).\n";
        return 2;
    }
    vector<double> c(n), b(m);
    vector<vector<double>> A(m, vector<double>(n));
    for (double& value : c) {
        if (!(cin >> value)) { cerr << "Input error: missing objective coefficient.\n"; return 2; }
    }
    for (auto& row : A) for (double& value : row) {
        if (!(cin >> value)) { cerr << "Input error: missing matrix coefficient.\n"; return 2; }
    }
    for (double& value : b) {
        if (!(cin >> value)) { cerr << "Input error: missing right-hand side value.\n"; return 2; }
    }
    string extra;
    if (cin >> extra) {
        cerr << "Input error: unexpected data after the right-hand side vector.\n";
        return 2;
    }
    for (double value : c) {
        if (!isInteger(value)) {
            cerr << "Input error: objective coefficients must be integers.\n";
            return 2;
        }
    }
    for (const auto& row : A) for (double value : row) {
        if (!isInteger(value)) {
            cerr << "Input error: matrix coefficients must be integers.\n";
            return 2;
        }
    }
    for (double value : b) {
        if (!isInteger(value)) {
            cerr << "Input error: right-hand side values must be integers.\n";
            return 2;
        }
    }

    cout << "Gomory Cutting Plane Method\n";
    cout << "===========================\n";
    cout << "Problem: maximize c^T x, subject to A x = b, x >= 0, x integer\n";
    cout << "Variables: " << n << ", constraints: " << m << "\n";
    cout << "Output mode: " << (detailed ? "DETAILED (every simplex phase and pivot)" : "BRIEF") << "\n";

    try {
        vector<vector<double>> modelA = A;
        vector<double> modelB = b;
        vector<double> objective = c;
        for (int stage = 1; stage <= MAX_GOMORY_CUTS; ++stage) {
            cout << "\n\n############################################################\n";
            cout << "GOMORY ITERATION " << stage << "\n";
            cout << "############################################################\n";
            Simplex simplex(modelA, modelB, (int)objective.size());
            LPSolution result = simplex.solve(objective, detailed);
            if (result.status == SolveStatus::Infeasible) {
                cout << "\nResult: the current relaxation is infeasible; the integer problem has no feasible plan.\n";
                return 0;
            }
            if (result.status == SolveStatus::Unbounded) {
                cout << "\nResult: LP relaxation is unbounded. The integer problem requires a separate unboundedness check.\n";
                return 0;
            }

            cout << "\nLP optimum after " << stage - 1 << " cut(s):\n  ";
            vector<double> originalX(result.x.begin(), result.x.begin() + n);
            printVector(originalX);
            cout << "\n  F(x) = " << fixed << setprecision(6) << result.value << '\n';
            cout << "  Basis B = (";
            for (int i = 0; i < (int)result.basis.size(); ++i) {
                if (i) cout << ", ";
                cout << "x" << result.basis[i] + 1;
            }
            cout << ")\n";

            int fractionalRow = -1;
            for (int i = 0; i < (int)result.basis.size(); ++i) {
                int basicVariable = result.basis[i];
                if (basicVariable < (int)objective.size() && !isInteger(result.rhs[i])) {
                    fractionalRow = i;
                    break;
                }
            }
            if (fractionalRow == -1) {
                cout << "\nAll integer-constrained variables have integer values.\n";
                cout << "Optimal integer plan: ";
                printVector(originalX);
                cout << "\nMaximum objective value: " << fixed << setprecision(6) << result.value << '\n';
                return 0;
            }

            int basicVariable = result.basis[fractionalRow];
            double rhsFraction = fractionalPart(result.rhs[fractionalRow]);
            vector<int> integerNonbasic;
            vector<double> cutCoefficients;
            for (int variable : result.nonbasic) {
                if (variable >= (int)objective.size()) continue;
                integerNonbasic.push_back(variable);
                cutCoefficients.push_back(fractionalPart(result.rows[fractionalRow][variable]));
            }

            cout << "\nGomory cut construction:\n";
            cout << "  Fractional basic variable: x" << basicVariable + 1
                 << " = " << fixed << setprecision(6) << result.rhs[fractionalRow]
                 << ", fractional part = " << rhsFraction << '\n';
            cout << "  Fractional parts of coefficients for nonbasic integer variables:\n";
            for (int i = 0; i < (int)integerNonbasic.size(); ++i)
                cout << "    x" << integerNonbasic[i] + 1 << ": " << cutCoefficients[i] << '\n';

            vector<long long> scaledCut = integerizeCut(cutCoefficients, rhsFraction);
            cout << "  Cut before scaling: sum(frac(a_j) * x_j) >= frac(b).\n";
            cout << "  Integer-scaled cut: ";
            bool hasTerm = false;
            for (int i = 0; i < (int)integerNonbasic.size(); ++i) {
                if (scaledCut[i] == 0) continue;
                if (hasTerm) cout << " + ";
                cout << scaledCut[i] << "*x" << integerNonbasic[i] + 1;
                hasTerm = true;
            }
            cout << " >= " << scaledCut.back() << '\n';

            int oldVariableCount = (int)objective.size();
            for (auto& row : modelA) row.push_back(0.0);
            vector<double> cutRow(oldVariableCount + 1, 0.0);
            for (int i = 0; i < (int)integerNonbasic.size(); ++i)
                cutRow[integerNonbasic[i]] = (double)scaledCut[i];
            cutRow[oldVariableCount] = -1.0;
            modelA.push_back(move(cutRow));
            modelB.push_back((double)scaledCut.back());
            objective.push_back(0.0);

            cout << "  Added as equality with integer slack x" << oldVariableCount + 1
                 << ": (cut left side) - x" << oldVariableCount + 1
                 << " = " << scaledCut.back() << ", x" << oldVariableCount + 1 << " >= 0.\n";
            cout << "  Re-solve the enlarged LP in the next iteration.\n";
        }
        cout << "\nStopped: exceeded the safety limit of " << MAX_GOMORY_CUTS << " Gomory cuts.\n";
    } catch (const exception& error) {
        cerr << "Solver error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
