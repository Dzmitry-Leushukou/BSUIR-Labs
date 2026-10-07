#pragma once

#include <vector>
#include <map>

struct Statistics {
    double mean;
    double variance;
    double std_dev;
};

struct ConfidenceInterval {
    double lower;
    double upper;
};

// Вычисление точечных оценок
Statistics calculate_statistics(const std::vector<double>& data);
Statistics calculate_statistics_discrete(const std::vector<int>& data);

// Теоретические характеристики
double exponential_mean(double lambda);
double exponential_variance(double lambda);

// Теоретические характеристики дискретной СВ
double discrete_mean(const std::map<int, double>& distribution);
double discrete_variance(const std::map<int, double>& distribution);

// Доверительные интервалы для матожидания (через распределение Стьюдента)
ConfidenceInterval confidence_interval_mean(const std::vector<double>& data, double alpha);
ConfidenceInterval confidence_interval_mean_discrete(const std::vector<int>& data, double alpha);

// Доверительные интервалы для дисперсии (через χ²)
ConfidenceInterval confidence_interval_variance(const std::vector<double>& data, double alpha);
ConfidenceInterval confidence_interval_variance_discrete(const std::vector<int>& data, double alpha);
