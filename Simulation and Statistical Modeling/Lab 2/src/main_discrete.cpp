#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/fl_ask.H>
#include "histogram_widget.hpp"
#include "generators.hpp"
#include "stats.hpp"
#include "tests.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <map>

// Глобальные виджеты
static Fl_Multiline_Input* input_distribution = nullptr;
static Fl_Input* input_n = nullptr;
static Fl_Input* input_alpha = nullptr;
static Fl_Input* input_seed = nullptr;
static HistogramWidget* histogram = nullptr;
static Fl_Text_Display* output_display = nullptr;
static Fl_Text_Buffer* output_buffer = nullptr;

// Парсинг таблицы распределения
static bool parse_distribution(const std::string& text, std::map<int, double>& dist, std::string& error) {
    dist.clear();
    std::istringstream iss(text);
    std::string token;
    double sum = 0.0;

    while (std::getline(iss, token, ',')) {
        size_t colon_pos = token.find(':');
        if (colon_pos == std::string::npos) {
            error = "Неверный формат. Используйте: значение:вероятность";
            return false;
        }

        try {
            int value = std::stoi(token.substr(0, colon_pos));
            double prob = std::stod(token.substr(colon_pos + 1));

            if (prob < 0) {
                error = "Вероятность не может быть отрицательной";
                return false;
            }

            dist[value] = prob;
            sum += prob;
        } catch (...) {
            error = "Ошибка парсинга чисел";
            return false;
        }
    }

    if (std::abs(sum - 1.0) > 1e-9) {
        error = "Сумма вероятностей должна быть равна 1 (текущая: " + std::to_string(sum) + ")";
        return false;
    }

    return true;
}

static void generate_callback(Fl_Widget*, void*) {
    try {
        // Парсинг распределения
        std::map<int, double> distribution;
        std::string error;
        if (!parse_distribution(input_distribution->value(), distribution, error)) {
            fl_alert("%s", error.c_str());
            return;
        }

        // Чтение параметров
        size_t n = std::stoul(input_n->value());
        double alpha = std::stod(input_alpha->value());

        if (n < 10) {
            fl_alert("Объём выборки должен быть не менее 10");
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
        auto data = gen.generate_discrete(distribution, n);

        // Статистики
        Statistics stats = calculate_statistics_discrete(data);
        double theo_mean = discrete_mean(distribution);
        double theo_var = discrete_variance(distribution);

        // Доверительные интервалы
        ConfidenceInterval ci_mean = confidence_interval_mean_discrete(data, alpha);
        ConfidenceInterval ci_var = confidence_interval_variance_discrete(data, alpha);

        // Подсчет частот
        std::map<int, int> freq_counts;
        for (int val : data) {
            freq_counts[val]++;
        }

        std::vector<int> values;
        std::vector<double> frequencies;
        std::vector<double> theoretical_probs;

        for (const auto& [val, prob] : distribution) {
            values.push_back(val);
            frequencies.push_back(static_cast<double>(freq_counts[val]) / n);
            theoretical_probs.push_back(prob);
        }

        histogram->set_bar_chart_data(values, frequencies, theoretical_probs);

        // Критерий согласия
        ChiSquareResult chi2 = chi_square_test_discrete(data, distribution, alpha);

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
        oss << "Таблица значений:\n";
        oss << std::setw(15) << "Значение" << std::setw(15) << "Наблюдаемая" << std::setw(15) << "Ожидаемая" << "\n";
        for (const auto& interval : chi2.intervals) {
            oss << std::setw(15) << static_cast<int>(interval.lower)
                << std::setw(15) << interval.observed
                << std::setw(15) << interval.expected << "\n";
        }
        oss << "\nСтатистика χ²: " << chi2.statistic << "\n";
        oss << "Критическое значение: " << chi2.critical_value << "\n";
        oss << "p-value: " << chi2.p_value << "\n";
        oss << "Степени свободы: " << chi2.degrees_of_freedom << "\n";
        oss << "Вывод: гипотеза " << (chi2.reject_null ? "ОТВЕРГАЕТСЯ" : "НЕ ОТВЕРГАЕТСЯ") << "\n";

        output_buffer->text(oss.str().c_str());

    } catch (const std::exception& e) {
        fl_alert("Ошибка: %s", e.what());
    }
}

int main(int argc, char** argv) {
    Fl_Double_Window* window = new Fl_Double_Window(1200, 700, "Лаб 2: Дискретное распределение");

    // Левая панель - параметры
    int x_offset = 10;
    int y_offset = 10;
    int label_w = 150;
    int input_w = 120;
    int line_h = 30;

    new Fl_Box(x_offset, y_offset, label_w + input_w, line_h, "Распределение (значение:вероятность):");
    y_offset += line_h;

    input_distribution = new Fl_Multiline_Input(x_offset, y_offset, label_w + input_w, 80);
    input_distribution->value("1:0.1, 2:0.2, 3:0.4, 4:0.2, 5:0.1");
    y_offset += 85;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Объём выборки (n):");
    input_n = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_n->value("1000");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Уровень знач. (α):");
    input_alpha = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    input_alpha->value("0.05");
    y_offset += line_h + 5;

    new Fl_Box(x_offset, y_offset, label_w, line_h, "Seed (опционально):");
    input_seed = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
    y_offset += line_h + 10;

    Fl_Button* btn_generate = new Fl_Button(x_offset, y_offset, 270, 40, "Сгенерировать");
    btn_generate->callback(generate_callback);

    // Виджет столбчатой диаграммы
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
