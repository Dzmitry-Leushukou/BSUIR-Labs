#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>
#include "histogram_widget.hpp"
#include "generators.hpp"
#include "stats.hpp"
#include "tests.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

// Глобальные виджеты
static Fl_Input* input_lambda = nullptr;
static Fl_Input* input_n = nullptr;
static Fl_Input* input_k = nullptr;
static Fl_Input* input_alpha = nullptr;
static Fl_Input* input_seed = nullptr;
static HistogramWidget* histogram = nullptr;
static Fl_Text_Display* output_display = nullptr;
static Fl_Text_Buffer* output_buffer = nullptr;

// Формула Стёрджесса
static int sturges_formula(size_t n) {
    return static_cast<int>(1 + 3.322 * std::log10(n));
}

static void generate_callback(Fl_Widget*, void*) {
    try {
        // Чтение параметров
        double lambda = std::stod(input_lambda->value());
        size_t n = std::stoul(input_n->value());
        int k = input_k->value()[0] ? std::stoi(input_k->value()) : sturges_formula(n);
        double alpha = std::stod(input_alpha->value());

        if (lambda <= 0) {
            fl_alert("Параметр λ должен быть положительным");
            return;
        }
        if (n < 10) {
            fl_alert("Объём выборки должен быть не менее 10");
            return;
        }
        if (k < 2) {
            fl_alert("Число интервалов должно быть не менее 2");
            return;
        }
        if (alpha <= 0 || alpha >= 1) {
            fl_alert("Уровень значимости должен быть в диапазоне (0, 1)");
            return;
        }

        // Генератор
        RandomGenerator gen;
        if (input_seed->value()[0]) {
            unsigned int seed = std::stoul(input_seed->value());
            gen.set_seed(seed);
        }

        // Генерация выборки
        auto data = gen.generate_exponential(lambda, n);

        // Статистики
        Statistics stats = calculate_statistics(data);
        double theo_mean = exponential_mean(lambda);
        double theo_var = exponential_variance(lambda);

        // Доверительные интервалы
        ConfidenceInterval ci_mean = confidence_interval_mean(data, alpha);
        ConfidenceInterval ci_var = confidence_interval_variance(data, alpha);

        // Гистограмма
        double min_val = *std::min_element(data.begin(), data.end());
        double max_val = *std::max_element(data.begin(), data.end());
        double bin_width = (max_val - min_val) / k;

        std::vector<double> bin_edges;
        std::vector<int> bin_counts(k, 0);

        for (int i = 0; i <= k; ++i) {
            bin_edges.push_back(min_val + i * bin_width);
        }

        for (double x : data) {
            int bin = static_cast<int>((x - min_val) / bin_width);
            if (bin >= k) bin = k - 1;
            bin_counts[bin]++;
        }

        // Нормировка на плотность
        std::vector<double> heights;
        for (int count : bin_counts) {
            heights.push_back(count / (n * bin_width));
        }

        // Функция плотности
        auto density = [lambda](double x) -> double {
            return lambda * std::exp(-lambda * x);
        };

        histogram->set_histogram_data(bin_edges, heights, density);

        // Критерии согласия
        ChiSquareResult chi2 = chi_square_test_exponential(data, lambda, k, alpha);
        KolmogorovResult kolm = kolmogorov_test_exponential(data, lambda, alpha);

        // Формирование вывода
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);

        oss << "=== ТОЧЕЧНЫЕ ОЦЕНКИ ===\n";
        oss << "Выборочное среднее:        " << stats.mean << " (теор: " << theo_mean << ")\n";
        oss << "Несмещённая дисперсия:     " << stats.variance << " (теор: " << theo_var << ")\n";
        oss << "Среднеквадратическое откл.: " << stats.std_dev << "\n\n";

        oss << "=== ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ (уровень " << (1 - alpha) << ") ===\n";
        oss << "Матожидание: [" << ci_mean.lower << ", " << ci_mean.upper << "]\n";
        oss << "Дисперсия:   [" << ci_var.lower << ", " << ci_var.upper << "]\n\n";

        oss << "=== КРИТЕРИЙ ХИ-КВАДРАТ ПИРСОНА ===\n";
        oss << "Таблица интервалов:\n";
        oss << std::setw(15) << "Нижняя" << std::setw(15) << "Верхняя"
            << std::setw(15) << "Наблюдаемая" << std::setw(15) << "Ожидаемая" << "\n";
        for (const auto& interval : chi2.intervals) {
            oss << std::setw(15) << interval.lower
                << std::setw(15) << interval.upper
                << std::setw(15) << interval.observed
                << std::setw(15) << interval.expected << "\n";
        }
        oss << "\nСтатистика χ²: " << chi2.statistic << "\n";
        oss << "Критическое значение: " << chi2.critical_value << "\n";
        oss << "p-value: " << chi2.p_value << "\n";
        oss << "Степени свободы: " << chi2.degrees_of_freedom << "\n";
        oss << "Вывод: гипотеза " << (chi2.reject_null ? "ОТВЕРГАЕТСЯ" : "НЕ ОТВЕРГАЕТСЯ") << "\n\n";

        oss << "=== КРИТЕРИЙ КОЛМОГОРОВА ===\n";
        oss << "Статистика D: " << kolm.statistic << "\n";
        oss << "p-value: " << kolm.p_value << "\n";
        oss << "Вывод: гипотеза " << (kolm.reject_null ? "ОТВЕРГАЕТСЯ" : "НЕ ОТВЕРГАЕТСЯ") << "\n";

        output_buffer->text(oss.str().c_str());

    } catch (const std::exception& e) {
        fl_alert("Ошибка: %s", e.what());
    }
}

int main(int argc, char** argv) {
    Fl_Double_Window* window = new Fl_Double_Window(1200, 700, "Лаб 2: Экспоненциальное распределение");

    // Левая панель - параметры
    int x_offset = 10;
    int y_offset = 10;
    int label_w = 150;
    int input_w = 100;
    int line_h = 30;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "λ (lambda):");
    input_lambda = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_lambda->value("1.0");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Объём выборки (n):");
    input_n = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_n->value("1000");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Число интервалов (k):");
    input_k = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_k->value("");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Уровень знач. (α):");
    input_alpha = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_alpha->value("0.05");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Seed (опционально):");
    input_seed = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    y_offset += line_h + 10;

    Fl_Button* btn_generate = new Fl_Button(x_offset, y_offset, 250, 40, "Сгенерировать");
    btn_generate->callback(generate_callback);

    // Виджет гистограммы
    histogram = new HistogramWidget(300, 10, 880, 400, nullptr);

    // Текстовый вывод
    output_buffer = new Fl_Text_Buffer();
    output_display = new Fl_Text_Display(300, 420, 880, 270);
    output_display->buffer(output_buffer);
    output_display->textfont(FL_COURIER);
    output_display->textsize(12);

    window->end();
    window->resizable(window);
    window->show(argc, argv);

    return Fl::run();
}
