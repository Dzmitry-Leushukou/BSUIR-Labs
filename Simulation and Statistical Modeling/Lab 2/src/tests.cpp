#include "tests.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <boost/math/distributions/chi_squared.hpp>

// Вычисление CDF экспоненциального распределения
static double exponential_cdf(double x, double lambda) {
    if (x < 0) return 0.0;
    return 1.0 - std::exp(-lambda * x);
}

// P-value для критерия Колмогорова (асимптотический ряд)
static double kolmogorov_p_value(double D, size_t n) {
    double lambda = (std::sqrt(n) + 0.12 + 0.11 / std::sqrt(n)) * D;

    // Асимптотический ряд Колмогорова: P(K <= lambda) = 1 - 2*sum((-1)^(k-1)*exp(-2*k^2*lambda^2))
    double sum = 0.0;
    for (int k = 1; k <= 100; ++k) {
        double term = std::pow(-1.0, k - 1) * std::exp(-2.0 * k * k * lambda * lambda);
        sum += term;
        if (std::abs(term) < 1e-10) break;
    }

    double p = 1.0 - 2.0 * sum;
    return std::max(0.0, std::min(1.0, p));
}

ChiSquareResult chi_square_test_exponential(
    const std::vector<double>& data,
    double lambda,
    int num_bins,
    double alpha
) {
    if (data.empty()) {
        return {{}, 0.0, 0.0, 1.0, 0, false};
    }

    size_t n = data.size();

    // Найти диапазон данных
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());

    // Создание интервалов
    std::vector<ChiSquareInterval> intervals;
    double bin_width = (max_val - min_val) / num_bins;

    // Подсчет наблюдаемых частот
    std::vector<int> observed_freq(num_bins, 0);
    for (double x : data) {
        int bin = static_cast<int>((x - min_val) / bin_width);
        if (bin >= num_bins) bin = num_bins - 1;
        observed_freq[bin]++;
    }

    // Вычисление ожидаемых частот
    std::vector<double> expected_freq(num_bins);
    for (int i = 0; i < num_bins; ++i) {
        double lower = min_val + i * bin_width;
        double upper = min_val + (i + 1) * bin_width;
        double prob = exponential_cdf(upper, lambda) - exponential_cdf(lower, lambda);
        expected_freq[i] = n * prob;

        intervals.push_back({lower, upper, observed_freq[i], expected_freq[i]});
    }

    // Объединение интервалов с малыми ожидаемыми частотами
    std::vector<ChiSquareInterval> merged_intervals;
    ChiSquareInterval current = intervals[0];

    for (size_t i = 1; i < intervals.size(); ++i) {
        if (current.expected < 5.0) {
            current.upper = intervals[i].upper;
            current.observed += intervals[i].observed;
            current.expected += intervals[i].expected;
        } else {
            merged_intervals.push_back(current);
            current = intervals[i];
        }
    }
    merged_intervals.push_back(current);

    // Вычисление статистики χ²
    double chi2_stat = 0.0;
    for (const auto& interval : merged_intervals) {
        if (interval.expected > 0) {
            double diff = interval.observed - interval.expected;
            chi2_stat += (diff * diff) / interval.expected;
        }
    }

    // Число степеней свободы: k - 1 (параметр lambda задан, не оценивается)
    int df = static_cast<int>(merged_intervals.size()) - 1;
    if (df < 1) df = 1;

    // Критическое значение и p-value
    boost::math::chi_squared chi2_dist(df);
    double critical_value = boost::math::quantile(chi2_dist, 1.0 - alpha);
    double p_value = 1.0 - boost::math::cdf(chi2_dist, chi2_stat);

    bool reject = chi2_stat > critical_value;

    return {merged_intervals, chi2_stat, critical_value, p_value, df, reject};
}

ChiSquareResult chi_square_test_discrete(
    const std::vector<int>& data,
    const std::map<int, double>& distribution,
    double alpha
) {
    if (data.empty()) {
        return {{}, 0.0, 0.0, 1.0, 0, false};
    }

    size_t n = data.size();

    // Подсчет наблюдаемых частот
    std::map<int, int> observed_counts;
    for (int x : data) {
        observed_counts[x]++;
    }

    // Создание интервалов (для дискретного распределения это отдельные значения)
    std::vector<ChiSquareInterval> intervals;
    for (const auto& [value, prob] : distribution) {
        int observed = observed_counts[value];
        double expected = n * prob;
        intervals.push_back({static_cast<double>(value), static_cast<double>(value), observed, expected});
    }

    // Объединение значений с малыми ожидаемыми частотами
    std::vector<ChiSquareInterval> merged_intervals;
    ChiSquareInterval current = intervals[0];

    for (size_t i = 1; i < intervals.size(); ++i) {
        if (current.expected < 5.0) {
            current.upper = intervals[i].upper;
            current.observed += intervals[i].observed;
            current.expected += intervals[i].expected;
        } else {
            merged_intervals.push_back(current);
            current = intervals[i];
        }
    }
    merged_intervals.push_back(current);

    // Вычисление статистики χ²
    double chi2_stat = 0.0;
    for (const auto& interval : merged_intervals) {
        if (interval.expected > 0) {
            double diff = interval.observed - interval.expected;
            chi2_stat += (diff * diff) / interval.expected;
        }
    }

    int df = static_cast<int>(merged_intervals.size()) - 1;
    if (df < 1) df = 1;

    boost::math::chi_squared chi2_dist(df);
    double critical_value = boost::math::quantile(chi2_dist, 1.0 - alpha);
    double p_value = 1.0 - boost::math::cdf(chi2_dist, chi2_stat);

    bool reject = chi2_stat > critical_value;

    return {merged_intervals, chi2_stat, critical_value, p_value, df, reject};
}

KolmogorovResult kolmogorov_test_exponential(
    const std::vector<double>& data,
    double lambda,
    double alpha
) {
    if (data.empty()) {
        return {0.0, 1.0, false};
    }

    std::vector<double> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    size_t n = sorted_data.size();
    double D = 0.0;

    // Вычисление D = max|Fn - F|
    for (size_t i = 0; i < n; ++i) {
        double x = sorted_data[i];
        double F_theoretical = exponential_cdf(x, lambda);

        // Fn слева от скачка: i/n
        double Fn_left = static_cast<double>(i) / n;
        // Fn справа от скачка: (i+1)/n
        double Fn_right = static_cast<double>(i + 1) / n;

        double D1 = std::abs(Fn_left - F_theoretical);
        double D2 = std::abs(Fn_right - F_theoretical);

        D = std::max({D, D1, D2});
    }

    double p_value = kolmogorov_p_value(D, n);
    bool reject = (p_value < alpha);

    return {D, p_value, reject};
}
