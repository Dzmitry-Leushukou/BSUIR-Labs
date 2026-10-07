#include "generators.hpp"
#include "stats.hpp"
#include "tests.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <map>

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== САМОПРОВЕРКА ЛАБОРАТОРНОЙ РАБОТЫ №2 ===\n\n";

    // Фиксированный seed для воспроизводимости
    const unsigned int seed = 12345;
    const size_t n = 1000000;
    const double alpha = 0.05;

    bool all_passed = true;

    // Тест 1: Экспоненциальное распределение
    std::cout << "Тест 1: Экспоненциальное распределение (λ=1.5, n=" << n << ")\n";
    {
        const double lambda = 1.5;
        RandomGenerator gen(seed);
        auto data = gen.generate_exponential(lambda, n);

        Statistics stats = calculate_statistics(data);
        double theo_mean = exponential_mean(lambda);
        double theo_var = exponential_variance(lambda);

        std::cout << "  Выборочное среднее: " << stats.mean << " (теор: " << theo_mean << ")\n";
        std::cout << "  Выборочная дисперсия: " << stats.variance << " (теор: " << theo_var << ")\n";

        double mean_error = std::abs(stats.mean - theo_mean) / theo_mean;
        double var_error = std::abs(stats.variance - theo_var) / theo_var;

        std::cout << "  Относительная ошибка среднего: " << (mean_error * 100) << "%\n";
        std::cout << "  Относительная ошибка дисперсии: " << (var_error * 100) << "%\n";

        if (mean_error < 0.01 && var_error < 0.01) {
            std::cout << "  ✓ PASSED: оценки близки к теоретическим значениям\n";
        } else {
            std::cout << "  ✗ FAILED: оценки слишком далеки от теоретических\n";
            all_passed = false;
        }

        // Критерий хи-квадрат
        int k = 50;
        ChiSquareResult chi2 = chi_square_test_exponential(data, lambda, k, alpha);
        std::cout << "  Критерий χ²: статистика=" << chi2.statistic
                  << ", p-value=" << chi2.p_value << "\n";

        if (!chi2.reject_null) {
            std::cout << "  ✓ PASSED: гипотеза не отвергается (p=" << chi2.p_value << ")\n";
        } else {
            std::cout << "  ✗ FAILED: гипотеза отвергается\n";
            all_passed = false;
        }

        // Критерий Колмогорова
        KolmogorovResult kolm = kolmogorov_test_exponential(data, lambda, alpha);
        std::cout << "  Критерий Колмогорова: D=" << kolm.statistic
                  << ", p-value=" << kolm.p_value << "\n";

        if (!kolm.reject_null) {
            std::cout << "  ✓ PASSED: гипотеза не отвергается (p=" << kolm.p_value << ")\n";
        } else {
            std::cout << "  ✗ FAILED: гипотеза отвергается\n";
            all_passed = false;
        }
    }

    std::cout << "\n";

    // Тест 2: Дискретное распределение
    std::cout << "Тест 2: Дискретное распределение (n=" << n << ")\n";
    {
        std::map<int, double> distribution = {
            {1, 0.1},
            {2, 0.15},
            {3, 0.25},
            {4, 0.3},
            {5, 0.2}
        };

        RandomGenerator gen(seed);
        auto data = gen.generate_discrete(distribution, n);

        Statistics stats = calculate_statistics_discrete(data);
        double theo_mean = discrete_mean(distribution);
        double theo_var = discrete_variance(distribution);

        std::cout << "  Выборочное среднее: " << stats.mean << " (теор: " << theo_mean << ")\n";
        std::cout << "  Выборочная дисперсия: " << stats.variance << " (теор: " << theo_var << ")\n";

        double mean_error = std::abs(stats.mean - theo_mean) / theo_mean;
        double var_error = std::abs(stats.variance - theo_var) / theo_var;

        std::cout << "  Относительная ошибка среднего: " << (mean_error * 100) << "%\n";
        std::cout << "  Относительная ошибка дисперсии: " << (var_error * 100) << "%\n";

        if (mean_error < 0.01 && var_error < 0.02) {
            std::cout << "  ✓ PASSED: оценки близки к теоретическим значениям\n";
        } else {
            std::cout << "  ✗ FAILED: оценки слишком далеки от теоретических\n";
            all_passed = false;
        }

        // Подсчет частот
        std::map<int, int> freq_counts;
        for (int val : data) {
            freq_counts[val]++;
        }

        std::cout << "  Частоты:\n";
        bool freq_ok = true;
        for (const auto& [val, prob] : distribution) {
            double observed_freq = static_cast<double>(freq_counts[val]) / n;
            double freq_error = std::abs(observed_freq - prob) / prob;
            std::cout << "    Значение " << val << ": наблюдаемая=" << observed_freq
                      << ", теоретическая=" << prob
                      << ", ошибка=" << (freq_error * 100) << "%\n";
            if (freq_error > 0.05) {
                freq_ok = false;
            }
        }

        if (freq_ok) {
            std::cout << "  ✓ PASSED: частоты близки к теоретическим\n";
        } else {
            std::cout << "  ✗ FAILED: некоторые частоты далеки от теоретических\n";
            all_passed = false;
        }

        // Критерий хи-квадрат
        ChiSquareResult chi2 = chi_square_test_discrete(data, distribution, alpha);
        std::cout << "  Критерий χ²: статистика=" << chi2.statistic
                  << ", p-value=" << chi2.p_value << "\n";

        if (!chi2.reject_null) {
            std::cout << "  ✓ PASSED: гипотеза не отвергается (p=" << chi2.p_value << ")\n";
        } else {
            std::cout << "  ✗ FAILED: гипотеза отвергается\n";
            all_passed = false;
        }
    }

    std::cout << "\n";
    std::cout << "=== ИТОГОВЫЙ РЕЗУЛЬТАТ: " << (all_passed ? "ВСЕ ТЕСТЫ ПРОЙДЕНЫ ✓" : "НЕКОТОРЫЕ ТЕСТЫ НЕ ПРОШЛИ ✗") << " ===\n";

    return all_passed ? 0 : 1;
}
