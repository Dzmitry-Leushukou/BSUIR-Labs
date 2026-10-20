#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

using Value = long long;

struct Problem {
    int itemCount;
    int capacity;
    vector<int> volume;
    vector<Value> cost;
};

struct Solution {
    vector<vector<Value>> optimum;
    vector<vector<unsigned char>> take;
    vector<int> selected;
    int usedCapacity = 0;
    Value totalValue = 0;
};

Problem readProblem(istream& input) {
    long long n = 0;
    long long capacity = 0;
    if (!(input >> n >> capacity))
        throw runtime_error("Input error: expected item count and backpack capacity: n B.");
    if (n <= 0 || n > numeric_limits<int>::max())
        throw runtime_error("Input error: n must be a positive integer.");
    if (capacity < 0 || capacity > numeric_limits<int>::max())
        throw runtime_error("Input error: B must be a non-negative integer.");

    Problem problem{static_cast<int>(n), static_cast<int>(capacity),
                    vector<int>(static_cast<size_t>(n)),
                    vector<Value>(static_cast<size_t>(n))};

    // Each of the next n lines contains one item: volume_i value_i.
    for (int i = 0; i < problem.itemCount; ++i) {
        long long volume = 0;
        long long value = 0;
        if (!(input >> volume >> value))
            throw runtime_error("Input error: expected n pairs volume_i value_i.");
        if (volume < 0 || volume > numeric_limits<int>::max())
            throw runtime_error("Input error: item volumes must be non-negative integers.");
        if (value < 0)
            throw runtime_error("Input error: item values must be non-negative integers.");
        problem.volume[i] = static_cast<int>(volume);
        problem.cost[i] = value;
    }

    string extra;
    if (input >> extra)
        throw runtime_error("Input error: unexpected data after the item list.");

    return problem;
}

Solution solveKnapsack(const Problem& problem) {
    const int n = problem.itemCount;
    const int B = problem.capacity;

    Solution solution;
    solution.optimum.assign(n + 1, vector<Value>(B + 1, 0));
    solution.take.assign(n + 1, vector<unsigned char>(B + 1, 0));

    // OPT(k, b) is the best value using the first k items and capacity b.
    for (int k = 1; k <= n; ++k) {
        const int volume = problem.volume[k - 1];
        const Value value = problem.cost[k - 1];
        for (int b = 0; b <= B; ++b) {
            const Value skip = solution.optimum[k - 1][b];
            solution.optimum[k][b] = skip;

            if (volume <= b) {
                const Value choose = solution.optimum[k - 1][b - volume] + value;
                // Strict comparison matches x(k,b) in the laboratory text.
                if (choose > skip) {
                    solution.optimum[k][b] = choose;
                    solution.take[k][b] = 1;
                }
            }
        }
    }

    int remaining = B;
    for (int k = n; k >= 1; --k) {
        if (!solution.take[k][remaining])
            continue;
        solution.selected.push_back(k);
        remaining -= problem.volume[k - 1];
        solution.usedCapacity += problem.volume[k - 1];
        solution.totalValue += problem.cost[k - 1];
    }
    reverse(solution.selected.begin(), solution.selected.end());
    return solution;
}

void printTable(const Problem& problem, const Solution& solution) {
    cout << "\nOPT(k, b) table (rows are k, columns are b):\n";
    cout << setw(8) << "k\\b";
    for (int b = 0; b <= problem.capacity; ++b)
        cout << setw(8) << b;
    cout << '\n';

    for (int k = 0; k <= problem.itemCount; ++k) {
        cout << setw(8) << k;
        for (int b = 0; b <= problem.capacity; ++b)
            cout << setw(8) << solution.optimum[k][b];
        cout << '\n';
    }

    cout << "\nx(k, b) table (1 means item k is selected):\n";
    cout << setw(8) << "k\\b";
    for (int b = 0; b <= problem.capacity; ++b)
        cout << setw(4) << b;
    cout << '\n';

    for (int k = 1; k <= problem.itemCount; ++k) {
        cout << setw(8) << k;
        for (int b = 0; b <= problem.capacity; ++b)
            cout << setw(4) << static_cast<int>(solution.take[k][b]);
        cout << '\n';
    }
}

void printResult(const Problem& problem, const Solution& solution) {
    cout << "\n========== RESULT ==========\n";
    cout << "Maximum value: " << solution.optimum[problem.itemCount][problem.capacity] << '\n';
    cout << "Used capacity: " << solution.usedCapacity << " / " << problem.capacity << '\n';
    cout << "Selected items (1-based): ";
    if (solution.selected.empty()) {
        cout << "none";
    } else {
        for (size_t i = 0; i < solution.selected.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << solution.selected[i];
        }
    }
    cout << '\n';

    cout << "Decision vector x = (";
    vector<unsigned char> selected(problem.itemCount + 1, 0);
    for (int item : solution.selected) selected[item] = 1;
    for (int i = 1; i <= problem.itemCount; ++i) {
        if (i != 1) cout << ", ";
        cout << static_cast<int>(selected[i]);
    }
    cout << ")\n";
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool detailed = false;
    string filename;
    for (int i = 1; i < argc; ++i) {
        const string argument = argv[i];
        if (argument == "--detailed") {
            detailed = true;
        } else if (filename.empty()) {
            filename = argument;
        } else {
            cerr << "Usage: " << argv[0] << " [--detailed] [input_file]\n";
            return 2;
        }
    }

    ifstream inputFile;
    if (!filename.empty()) {
        inputFile.open(filename);
        if (!inputFile) {
            cerr << "Input error: cannot open file: " << filename << '\n';
            return 2;
        }
        cin.rdbuf(inputFile.rdbuf());
    }

    try {
        const Problem problem = readProblem(cin);
        const Solution solution = solveKnapsack(problem);

        cout << "0/1 Knapsack - Dynamic Programming\n";
        cout << "=================================\n";
        cout << "Items: " << problem.itemCount << ", capacity: " << problem.capacity << '\n';
        cout << "Input format: n B followed by n pairs volume_i value_i\n";
        cout << "Output mode: " << (detailed ? "DETAILED" : "BRIEF") << '\n';

        if (detailed)
            printTable(problem, solution);
        printResult(problem, solution);
    } catch (const exception& error) {
        cerr << error.what() << '\n';
        return 2;
    }

    return 0;
}

