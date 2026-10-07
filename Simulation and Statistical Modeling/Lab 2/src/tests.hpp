#pragma once

#include <vector>
#include <map>

struct ChiSquareInterval {
    double lower;
    double upper;
    int observed;
    double expected;
};

struct ChiSquareResult {
    std::vector<ChiSquareInterval> intervals;
    double statistic;
    double critical_value;
    double p_value;
    int degrees_of_freedom;
    bool reject_null;
};

struct KolmogorovResult {
    double statistic;
    double p_value;
    bool reject_null;
};

// Критерий χ² Пирсона для экспоненциального распределения
ChiSquareResult chi_square_test_exponential(
    const std::vector<double>& data,
    double lambda,
    int num_bins,
    double alpha
);

// Критерий χ² Пирсона для дискретного распределения
ChiSquareResult chi_square_test_discrete(
    const std::vector<int>& data,
    const std::map<int, double>& distribution,
    double alpha
);

// Критерий Колмогорова для экспоненциального распределения
KolmogorovResult kolmogorov_test_exponential(
    const std::vector<double>& data,
    double lambda,
    double alpha
);
