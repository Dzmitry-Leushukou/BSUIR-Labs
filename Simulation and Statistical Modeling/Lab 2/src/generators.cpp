#include "generators.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

RandomGenerator::RandomGenerator(unsigned int seed)
    : gen_(seed), uniform_(0.0, 1.0) {
}

void RandomGenerator::set_seed(unsigned int seed) {
    gen_.seed(seed);
}

std::vector<double> RandomGenerator::generate_exponential(double lambda, size_t n) {
    std::vector<double> result;
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        double u = uniform_(gen_);
        // Используем 1-u, чтобы избежать log(0)
        double y = -std::log(1.0 - u) / lambda;
        result.push_back(y);
    }

    return result;
}

std::vector<int> RandomGenerator::generate_discrete(const std::map<int, double>& distribution, size_t n) {
    // Построение кумулятивных сумм
    std::vector<int> values;
    std::vector<double> cumulative;
    double sum = 0.0;

    for (const auto& [value, prob] : distribution) {
        values.push_back(value);
        sum += prob;
        cumulative.push_back(sum);
    }

    std::vector<int> result;
    result.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        double u = uniform_(gen_);
        // Бинарный поиск: находим первый элемент >= u
        auto it = std::upper_bound(cumulative.begin(), cumulative.end(), u);
        size_t index = std::distance(cumulative.begin(), it);
        result.push_back(values[index]);
    }

    return result;
}
