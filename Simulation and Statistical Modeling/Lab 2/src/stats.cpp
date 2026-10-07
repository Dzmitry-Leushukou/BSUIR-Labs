#include "stats.hpp"
#include <cmath>
#include <numeric>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>

Statistics calculate_statistics(const std::vector<double>& data) {
    if (data.empty()) {
        return {0.0, 0.0, 0.0};
    }

    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();

    double sq_sum = 0.0;
    for (double x : data) {
        sq_sum += (x - mean) * (x - mean);
    }

    // Несмещённая дисперсия
    double variance = (data.size() > 1) ? sq_sum / (data.size() - 1) : 0.0;
    double std_dev = std::sqrt(variance);

    return {mean, variance, std_dev};
}

Statistics calculate_statistics_discrete(const std::vector<int>& data) {
    if (data.empty()) {
        return {0.0, 0.0, 0.0};
    }

    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();

    double sq_sum = 0.0;
    for (int x : data) {
        sq_sum += (x - mean) * (x - mean);
    }

    double variance = (data.size() > 1) ? sq_sum / (data.size() - 1) : 0.0;
    double std_dev = std::sqrt(variance);

    return {mean, variance, std_dev};
}

double exponential_mean(double lambda) {
    return 1.0 / lambda;
}

double exponential_variance(double lambda) {
    return 1.0 / (lambda * lambda);
}

double discrete_mean(const std::map<int, double>& distribution) {
    double mean = 0.0;
    for (const auto& [value, prob] : distribution) {
        mean += value * prob;
    }
    return mean;
}

double discrete_variance(const std::map<int, double>& distribution) {
    double mean = discrete_mean(distribution);
    double variance = 0.0;
    for (const auto& [value, prob] : distribution) {
        variance += (value - mean) * (value - mean) * prob;
    }
    return variance;
}

ConfidenceInterval confidence_interval_mean(const std::vector<double>& data, double alpha) {
    if (data.size() < 2) {
        return {0.0, 0.0};
    }

    Statistics stats = calculate_statistics(data);
    size_t n = data.size();

    // Распределение Стьюдента с n-1 степенями свободы
    boost::math::students_t dist(n - 1);
    double t_crit = boost::math::quantile(dist, 1.0 - alpha / 2.0);

    double margin = t_crit * stats.std_dev / std::sqrt(n);

    return {stats.mean - margin, stats.mean + margin};
}

ConfidenceInterval confidence_interval_mean_discrete(const std::vector<int>& data, double alpha) {
    if (data.size() < 2) {
        return {0.0, 0.0};
    }

    Statistics stats = calculate_statistics_discrete(data);
    size_t n = data.size();

    boost::math::students_t dist(n - 1);
    double t_crit = boost::math::quantile(dist, 1.0 - alpha / 2.0);

    double margin = t_crit * stats.std_dev / std::sqrt(n);

    return {stats.mean - margin, stats.mean + margin};
}

ConfidenceInterval confidence_interval_variance(const std::vector<double>& data, double alpha) {
    if (data.size() < 2) {
        return {0.0, 0.0};
    }

    Statistics stats = calculate_statistics(data);
    size_t n = data.size();

    // Распределение χ² с n-1 степенями свободы
    boost::math::chi_squared dist(n - 1);
    double chi2_lower = boost::math::quantile(dist, alpha / 2.0);
    double chi2_upper = boost::math::quantile(dist, 1.0 - alpha / 2.0);

    double lower = (n - 1) * stats.variance / chi2_upper;
    double upper = (n - 1) * stats.variance / chi2_lower;

    return {lower, upper};
}

ConfidenceInterval confidence_interval_variance_discrete(const std::vector<int>& data, double alpha) {
    if (data.size() < 2) {
        return {0.0, 0.0};
    }

    Statistics stats = calculate_statistics_discrete(data);
    size_t n = data.size();

    boost::math::chi_squared dist(n - 1);
    double chi2_lower = boost::math::quantile(dist, alpha / 2.0);
    double chi2_upper = boost::math::quantile(dist, 1.0 - alpha / 2.0);

    double lower = (n - 1) * stats.variance / chi2_upper;
    double upper = (n - 1) * stats.variance / chi2_lower;

    return {lower, upper};
}
