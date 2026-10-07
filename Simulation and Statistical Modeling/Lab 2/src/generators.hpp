#pragma once

#include <vector>
#include <random>
#include <map>

class RandomGenerator {
public:
    explicit RandomGenerator(unsigned int seed = std::random_device{}());

    // Метод обратных функций для экспоненциального распределения
    std::vector<double> generate_exponential(double lambda, size_t n);

    // Дискретная случайная величина (метод разбиения отрезка)
    std::vector<int> generate_discrete(const std::map<int, double>& distribution, size_t n);

    void set_seed(unsigned int seed);

private:
    std::mt19937 gen_;
    std::uniform_real_distribution<double> uniform_;
};
