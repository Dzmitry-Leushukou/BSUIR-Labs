#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

using namespace std;

using Cost = long long;
const long double EPS = 1e-10L;

struct AssignmentResult {
    vector<int> columnForRow;
    vector<long double> rowPotential;
    vector<long double> columnPotential;
    Cost totalCost = 0;
    int iterations = 0;
};

bool augment(int row, const vector<vector<bool>>& equality,
             vector<bool>& visitedColumns, vector<int>& matchedRow) {
    for (int column = 0; column < static_cast<int>(equality.size()); ++column) {
        if (!equality[row][column] || visitedColumns[column]) continue;
        visitedColumns[column] = true;
        if (matchedRow[column] == -1 ||
            augment(matchedRow[column], equality, visitedColumns, matchedRow)) {
            matchedRow[column] = row;
            return true;
        }
    }
    return false;
}

AssignmentResult hungarian(const vector<vector<Cost>>& cost, bool detailed) {
    const int n = static_cast<int>(cost.size());
    AssignmentResult result;
    result.columnForRow.assign(n, -1);
    result.rowPotential.assign(n, 0);
    result.columnPotential.assign(n, 0);
    for (int column = 0; column < n; ++column) {
        Cost minimum = cost[0][column];
        for (int row = 1; row < n; ++row)
            minimum = min(minimum, cost[row][column]);
        result.columnPotential[column] = minimum;
    }
    if (detailed) {
        cout << "Step 1. Initial dual potentials\n";
        cout << fixed << setprecision(4);
        for (int row = 0; row < n; ++row)
            cout << "alpha[" << row + 1 << "] = 0\n";
        for (int column = 0; column < n; ++column)
            cout << "beta[" << column + 1 << "] = "
                 << static_cast<double>(result.columnPotential[column]) << '\n';
    }

    while (true) {
        ++result.iterations;
        if (detailed)
            cout << "\nIteration " << result.iterations << "\nStep 2. Build J= and J<\n";
        vector<vector<bool>> equality(n, vector<bool>(n, false));
        bool first = true;
        if (detailed) cout << "J= : ";
        for (int row = 0; row < n; ++row)
            for (int column = 0; column < n; ++column) {
                equality[row][column] =
                    fabsl(result.rowPotential[row] + result.columnPotential[column]
                          - cost[row][column]) <= EPS;
                if (detailed && equality[row][column]) {
                    if (!first) cout << ", ";
                    cout << "(" << row + 1 << "," << column + 1 << ")";
                    first = false;
                }
            }
        if (detailed && first) cout << "empty";
        if (detailed) {
            cout << "\nJ< : ";
            first = true;
            for (int row = 0; row < n; ++row) {
                for (int column = 0; column < n; ++column) {
                    if (equality[row][column]) continue;
                    if (!first) cout << ", ";
                    cout << "(" << row + 1 << "," << column + 1 << ")";
                    first = false;
                }
            }
            if (first) cout << "empty";
            cout << '\n';
        }

        if (detailed) cout << "Step 3-4. Build G and find a maximum matching\n";
        vector<int> matchedRow(n, -1);
        int matchingSize = 0;
        for (int row = 0; row < n; ++row) {
            vector<bool> visitedColumns(n, false);
            if (augment(row, equality, visitedColumns, matchedRow))
                ++matchingSize;
        }

        if (detailed) {
            cout << "\nMaximum matching M (size " << matchingSize << "): ";
            first = true;
            for (int column = 0; column < n; ++column) {
                if (matchedRow[column] == -1) continue;
                if (!first) cout << ", ";
                cout << "(" << matchedRow[column] + 1 << "," << column + 1 << ")";
                first = false;
            }
            cout << '\n';
        }

        if (matchingSize == n) {
            if (detailed) cout << "Step 5. |M| = n; the assignment is optimal.\n";
            for (int column = 0; column < n; ++column)
                result.columnForRow[matchedRow[column]] = column;
            break;
        }

        if (detailed)
            cout << "Step 6-7. Traverse the residual graph from unmatched rows; form I* and J*\n";
        vector<bool> reachableRows(n, false), reachableColumns(n, false);
        queue<int> pendingRows;
        vector<int> columnForRow(n, -1);
        for (int column = 0; column < n; ++column)
            if (matchedRow[column] != -1)
                columnForRow[matchedRow[column]] = column;
        for (int row = 0; row < n; ++row) {
            if (columnForRow[row] == -1) {
                reachableRows[row] = true;
                pendingRows.push(row);
            }
        }

        while (!pendingRows.empty()) {
            const int row = pendingRows.front();
            pendingRows.pop();
            for (int column = 0; column < n; ++column) {
                if (!equality[row][column] || columnForRow[row] == column ||
                    reachableColumns[column])
                    continue;
                reachableColumns[column] = true;
                if (matchedRow[column] != -1 && !reachableRows[matchedRow[column]]) {
                    reachableRows[matchedRow[column]] = true;
                    pendingRows.push(matchedRow[column]);
                }
            }
        }

        long double theta = numeric_limits<long double>::infinity();
        for (int row = 0; row < n; ++row) {
            if (!reachableRows[row]) continue;
            for (int column = 0; column < n; ++column) {
                if (reachableColumns[column]) continue;
                const long double slack = cost[row][column]
                                        - result.rowPotential[row]
                                        - result.columnPotential[column];
                theta = min(theta, slack / 2);
            }
        }
        if (!isfinite(theta) || theta <= EPS)
            throw runtime_error("Could not find a positive dual adjustment.");
        if (detailed) {
            cout << "I*: ";
            bool first = true;
            for (int row = 0; row < n; ++row) {
                if (!reachableRows[row]) continue;
                if (!first) cout << ", ";
                cout << row + 1;
                first = false;
            }
            cout << "\nJ*: ";
            first = true;
            for (int column = 0; column < n; ++column) {
                if (!reachableColumns[column]) continue;
                if (!first) cout << ", ";
                cout << column + 1;
                first = false;
            }
            if (first) cout << "empty";
            cout << "\nStep 8. Adjustment directions: alpha_i = +1 on I*, -1 outside; "
                    "beta_j = -1 on J*, +1 outside\n";
            cout << "Step 9. theta = min((c_ij - alpha_i - beta_j) / 2) = "
                 << fixed << setprecision(4)
                 << static_cast<double>(theta) << '\n';
        }

        if (detailed) cout << "Step 10-11. Update the dual potentials\n";
        for (int row = 0; row < n; ++row)
            result.rowPotential[row] += theta * (reachableRows[row] ? 1 : -1);
        for (int column = 0; column < n; ++column)
            result.columnPotential[column] += theta * (reachableColumns[column] ? -1 : 1);
        if (detailed) {
            for (int row = 0; row < n; ++row)
                cout << "alpha[" << row + 1 << "] = "
                     << static_cast<double>(result.rowPotential[row]) << '\n';
            for (int column = 0; column < n; ++column)
                cout << "beta[" << column + 1 << "] = "
                     << static_cast<double>(result.columnPotential[column]) << '\n';
        }
    }

    for (int row = 0; row < n; ++row)
        result.totalCost += cost[row][result.columnForRow[row]];
    return result;
}

bool readProblem(istream& input, vector<vector<Cost>>& cost, string& error) {
    int n;
    if (!(input >> n)) {
        error = "Could not read the matrix size.";
        return false;
    }
    if (n < 1 || n > 500) {
        error = "The matrix size must be between 1 and 500.";
        return false;
    }

    cost.assign(n, vector<Cost>(n));
    for (int row = 0; row < n; ++row) {
        for (int column = 0; column < n; ++column) {
            if (!(input >> cost[row][column])) {
                error = "The input must contain exactly n rows with n integer costs each.";
                return false;
            }
            if (cost[row][column] < -1000000000000LL ||
                cost[row][column] > 1000000000000LL) {
                error = "Each cost must be between -10^12 and 10^12.";
                return false;
            }
        }
    }

    string extra;
    if (input >> extra) {
        error = "Unexpected data after the n-by-n cost matrix.";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    bool detailed = false;
    string filename;
    for (int i = 1; i < argc; ++i) {
        const string argument = argv[i];
        if (argument == "--detailed") {
            detailed = true;
        } else if (filename.empty()) {
            filename = argument;
        } else {
            cerr << "Usage: " << argv[0] << " [input_file] [--detailed]\n";
            return 1;
        }
    }

    ifstream file;
    istream* input = &cin;
    if (!filename.empty()) {
        file.open(filename);
        if (!file) {
            cerr << "Could not open input file: " << filename << '\n';
            return 1;
        }
        input = &file;
    }

    vector<vector<Cost>> cost;
    string error;
    if (!readProblem(*input, cost, error)) {
        cerr << "Input error: " << error << '\n'
             << "Expected format: n, followed by n rows of n integer costs.\n";
        return 1;
    }

    cout << "Hungarian Algorithm - Assignment Problem\n"
         << "=========================================\n"
         << "Cost matrix size: " << cost.size() << " x " << cost.size() << '\n';
    const AssignmentResult result = hungarian(cost, detailed);
    if (detailed) {
        cout << "\nFinal dual potentials:\n";
        cout << fixed << setprecision(4);
        for (size_t i = 0; i < result.rowPotential.size(); ++i)
            cout << "alpha[" << i + 1 << "] = "
                 << static_cast<double>(result.rowPotential[i]) << '\n';
        for (size_t j = 0; j < result.columnPotential.size(); ++j)
            cout << "beta[" << j + 1 << "] = "
                 << static_cast<double>(result.columnPotential[j]) << '\n';
    }

    cout << "\nOptimal assignment:\n";
    for (size_t row = 0; row < result.columnForRow.size(); ++row) {
        const int column = result.columnForRow[row];
        cout << "Row " << row + 1 << " -> column " << column + 1
             << " (cost " << cost[row][column] << ")\n";
    }
    cout << "Minimum total cost: " << result.totalCost << '\n';
    return 0;
}
