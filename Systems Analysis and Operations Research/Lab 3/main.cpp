#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Solution {
    vector<vector<double>> best;
    vector<vector<int>> choice;
    vector<int> allocation;
};

static bool isInteger(double value) {
    return abs(value - round(value)) <= 1e-9;
}

static string formatNumber(double value) {
    ostringstream out;
    if (isInteger(value)) out << static_cast<long long>(llround(value));
    else out << fixed << setprecision(6) << value;
    return out.str();
}

static Solution solve(const vector<vector<double>>& profit) {
    const int agents = static_cast<int>(profit.size());
    const int resource = static_cast<int>(profit.front().size()) - 1;

    Solution result;
    result.best.assign(agents, vector<double>(resource + 1, 0.0));
    result.choice.assign(agents, vector<int>(resource + 1, 0));

    for (int q = 0; q <= resource; ++q) {
        result.best[0][q] = profit[0][q];
        result.choice[0][q] = q;
    }

    for (int p = 1; p < agents; ++p) {
        for (int q = 0; q <= resource; ++q) {
            double bestValue = -numeric_limits<double>::infinity();
            int bestChoice = 0;
            for (int assigned = 0; assigned <= q; ++assigned) {
                const double candidate = profit[p][assigned] + result.best[p - 1][q - assigned];
                // Scanning from zero keeps the first (smallest) allocation on ties.
                if (candidate > bestValue + 1e-9) {
                    bestValue = candidate;
                    bestChoice = assigned;
                }
            }
            result.best[p][q] = bestValue;
            result.choice[p][q] = bestChoice;
        }
    }

    result.allocation.assign(agents, 0);
    int remaining = resource;
    for (int p = agents - 1; p >= 0; --p) {
        result.allocation[p] = result.choice[p][remaining];
        remaining -= result.allocation[p];
    }
    return result;
}

static void printMatrix(const string& name, const vector<vector<double>>& matrix) {
    cout << "\n" << name << " (rows: agents, columns: q = 0..Q)\n";
    for (const auto& row : matrix) {
        for (double value : row) cout << setw(12) << formatNumber(value);
        cout << '\n';
    }
}

static void printChoiceMatrix(const vector<vector<int>>& matrix) {
    cout << "\nC (amount assigned to the current agent)\n";
    for (const auto& row : matrix) {
        for (int value : row) cout << setw(12) << value;
        cout << '\n';
    }
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool detailed = false;
    string filename;
    for (int i = 1; i < argc; ++i) {
        const string argument = argv[i];
        if (argument == "--detailed") detailed = true;
        else if (filename.empty()) filename = argument;
        else {
            cerr << "Usage: " << argv[0] << " [--detailed] [input_file]\n";
            return 2;
        }
    }

    ifstream input;
    if (!filename.empty()) {
        input.open(filename);
        if (!input) {
            cerr << "Cannot open input file: " << filename << '\n';
            return 2;
        }
        cin.rdbuf(input.rdbuf());
    }

    int agents = 0;
    int resource = 0;
    if (!(cin >> agents >> resource) || agents <= 0 || resource < 0) {
        cerr << "Input error: expected P > 0 and Q >= 0.\n";
        return 2;
    }

    vector<vector<double>> profit(agents, vector<double>(resource + 1));
    for (int p = 0; p < agents; ++p) {
        for (int q = 0; q <= resource; ++q) {
            if (!(cin >> profit[p][q]) || !isfinite(profit[p][q])) {
                cerr << "Input error: expected a P x (Q + 1) profit matrix.\n";
                return 2;
            }
        }
    }

    string extra;
    if (cin >> extra) {
        cerr << "Input error: unexpected data after the profit matrix.\n";
        return 2;
    }

    const Solution result = solve(profit);

    cout << "Resource Allocation Problem - Dynamic Programming\n";
    cout << "=================================================\n";
    cout << "Agents (P): " << agents << ", resource units (Q): " << resource << '\n';
    cout << "Input matrix A: profit for each agent and allocated amount\n";
    printMatrix("A", profit);
    printMatrix("B (maximum profit)", result.best);
    printChoiceMatrix(result.choice);

    cout << "\nOptimal allocation (backward pass):\n";
    int totalUnits = 0;
    for (int p = 0; p < agents; ++p) {
        cout << "  Agent " << p + 1 << ": " << result.allocation[p] << " unit(s)\n";
        totalUnits += result.allocation[p];
    }
    cout << "Total allocated: " << totalUnits << " of " << resource << '\n';
    cout << "Maximum total profit B(P, Q) = " << formatNumber(result.best.back().back()) << '\n';

    if (detailed) {
        cout << "\nDetailed recurrence values:\n";
        for (int p = 1; p <= agents; ++p) {
            for (int q = 0; q <= resource; ++q) {
                cout << "B(" << p << ", " << q << ") = "
                     << formatNumber(result.best[p - 1][q])
                     << ", C(" << p << ", " << q << ") = "
                     << result.choice[p - 1][q] << '\n';
            }
        }
    }
    return 0;
}
