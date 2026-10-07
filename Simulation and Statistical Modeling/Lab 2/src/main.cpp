#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
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
#include <optional>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <map>

static Fl_Input* input_lambda = nullptr;
static Fl_Input* input_n1 = nullptr;
static Fl_Input* input_k = nullptr;
static Fl_Input* input_alpha1 = nullptr;
static HistogramWidget* histogram1 = nullptr;
static Fl_Text_Display* output_display1 = nullptr;
static Fl_Text_Buffer* output_buffer1 = nullptr;

static Fl_Multiline_Input* input_distribution = nullptr;
static Fl_Input* input_n2 = nullptr;
static Fl_Input* input_alpha2 = nullptr;
static HistogramWidget* histogram2 = nullptr;
static Fl_Text_Display* output_display2 = nullptr;
static Fl_Text_Buffer* output_buffer2 = nullptr;

static int sturges_formula(size_t n) {
    return static_cast<int>(1 + 3.322 * std::log10(n));
}

void task1_generate_callback(Fl_Widget*, void*) {
    try {
        double lambda = std::stod(input_lambda->value());
        size_t n = std::stoull(input_n1->value());
        std::string k_str = input_k->value();
        int k = k_str.empty() ? sturges_formula(n) : std::stoi(k_str);
        double alpha = std::stod(input_alpha1->value());

        if (lambda <= 0) throw std::invalid_argument("λ должна быть > 0");
        if (n < 30) throw std::invalid_argument("n должно быть ≥ 30");
        if (k < 2) throw std::invalid_argument("k должно быть ≥ 2");
        if (alpha <= 0 || alpha >= 1) throw std::invalid_argument("α должна быть в (0, 1)");

        RandomGenerator rng(std::random_device{}());
        auto data = rng.generate_exponential(lambda, n);

        double min_val = *std::min_element(data.begin(), data.end());
        double max_val = *std::max_element(data.begin(), data.end());

        std::vector<double> frequencies_double(k, 0.0);
        double bin_width = (max_val - min_val) / k;

        for (double val : data) {
            int bin = std::min(static_cast<int>((val - min_val) / bin_width), k - 1);
            frequencies_double[bin]++;
        }

        std::vector<double> bin_centers(k);
        for (int i = 0; i < k; ++i) {
            bin_centers[i] = min_val + (i + 0.5) * bin_width;
        }

        histogram1->set_histogram_data(bin_centers, frequencies_double, [lambda, bin_width, n](double x) {
            return lambda * std::exp(-lambda * x) * bin_width * n;
        });
        histogram1->redraw();

        auto stats = calculate_statistics(data);
        double mean = stats.mean;
        double var = stats.variance;

        auto mean_ci = confidence_interval_mean(data, alpha);
        auto var_ci = confidence_interval_variance(data, alpha);

        double theo_mean = 1.0 / lambda;
        double theo_var = 1.0 / (lambda * lambda);

        auto chi2_result = chi_square_test_exponential(data, lambda, k, alpha);
        auto ks_result = kolmogorov_test_exponential(data, lambda, alpha);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);

        oss << "ЭКСПОНЕНЦИАЛЬНОЕ РАСПРЕДЕЛЕНИЕ: f(x) = λ·e^(-λx), λ = " << lambda << "\n";
        oss << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

        oss << "СТАТИСТИКИ ВЫБОРКИ (n = " << n << "):\n";
        oss << "  Выборочное среднее:      " << mean << "\n";
        oss << "  Теоретическое среднее:   " << theo_mean << "\n";
        oss << "  ДИ для среднего (" << (1-alpha)*100 << "%): ["
            << mean_ci.lower << ", " << mean_ci.upper << "]\n\n";

        oss << "  Выборочная дисперсия:    " << var << "\n";
        oss << "  Теоретическая дисперсия: " << theo_var << "\n";
        oss << "  ДИ для дисперсии (" << (1-alpha)*100 << "%): ["
            << var_ci.lower << ", " << var_ci.upper << "]\n\n";

        oss << "КРИТЕРИЙ χ² ПИРСОНА:\n";
        oss << "  Статистика χ²:           " << chi2_result.statistic << "\n";
        oss << "  Критическое значение:    " << chi2_result.critical_value << "\n";
        oss << "  P-value:                 " << chi2_result.p_value << "\n";
        oss << "  Степени свободы:         " << chi2_result.degrees_of_freedom << "\n";
        oss << "  Гипотеза H₀:             " << (chi2_result.reject_null ? "ОТВЕРГНУТА ❌" : "НЕ ОТВЕРГНУТА ✓") << "\n\n";

        oss << "КРИТЕРИЙ КОЛМОГОРОВА:\n";
        oss << "  Статистика D:            " << ks_result.statistic << "\n";
        oss << "  P-value:                 " << ks_result.p_value << "\n";
        oss << "  Гипотеза H₀:             " << (ks_result.reject_null ? "ОТВЕРГНУТА ❌" : "НЕ ОТВЕРГНУТА ✓") << "\n";

        output_buffer1->text(oss.str().c_str());

    } catch (const std::exception& e) {
        fl_alert("Ошибка: %s", e.what());
    }
}

void task2_generate_callback(Fl_Widget*, void*) {
    try {
        std::string dist_str = input_distribution->value();
        size_t n = std::stoull(input_n2->value());
        double alpha = std::stod(input_alpha2->value());

        if (n < 30) throw std::invalid_argument("n должно быть ≥ 30");
        if (alpha <= 0 || alpha >= 1) throw std::invalid_argument("α должна быть в (0, 1)");

        std::vector<double> values;
        std::vector<double> probabilities;

        std::istringstream iss(dist_str);
        std::string token;
        double prob_sum = 0.0;

        while (std::getline(iss, token, ',')) {
            size_t colon_pos = token.find(':');
            if (colon_pos == std::string::npos) {
                throw std::invalid_argument("Неверный формат. Используйте: значение:вероятность");
            }

            double val = std::stod(token.substr(0, colon_pos));
            double prob = std::stod(token.substr(colon_pos + 1));

            values.push_back(val);
            probabilities.push_back(prob);
            prob_sum += prob;
        }

        if (std::abs(prob_sum - 1.0) > 1e-9) {
            throw std::invalid_argument("Сумма вероятностей должна быть равна 1.0");
        }

        std::map<int, double> dist_map;
        for (size_t i = 0; i < values.size(); ++i) {
            dist_map[static_cast<int>(values[i])] = probabilities[i];
        }

        RandomGenerator rng(std::random_device{}());
        auto data = rng.generate_discrete(dist_map, n);

        std::map<int, int> frequency_map;
        for (int val : data) {
            frequency_map[val]++;
        }

        std::vector<double> unique_values;
        std::vector<double> frequencies_double;
        for (const auto& [val, freq] : frequency_map) {
            unique_values.push_back(static_cast<double>(val));
            frequencies_double.push_back(static_cast<double>(freq));
        }

        histogram2->set_histogram_data(unique_values, frequencies_double, [&dist_map, n](double x) {
            int ix = static_cast<int>(x);
            auto it = dist_map.find(ix);
            if (it != dist_map.end()) {
                return it->second * n;
            }
            return 0.0;
        });
        histogram2->redraw();

        auto stats = calculate_statistics_discrete(data);
        double mean = stats.mean;
        double var = stats.variance;

        double theo_mean = discrete_mean(dist_map);
        double theo_var = discrete_variance(dist_map);

        auto chi2_result = chi_square_test_discrete(data, dist_map, alpha);

        std::ostringstream dist_oss;
        dist_oss << "P(X=x): ";
        for (const auto& [val, prob] : dist_map) {
            dist_oss << "P(" << val << ")=" << prob << " ";
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);

        oss << "ДИСКРЕТНОЕ РАСПРЕДЕЛЕНИЕ\n";
        oss << dist_oss.str() << "\n";
        oss << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

        oss << "СТАТИСТИКИ ВЫБОРКИ (n = " << n << "):\n";
        oss << "  Выборочное среднее:      " << mean << "\n";
        oss << "  Теоретическое среднее:   " << theo_mean << "\n\n";

        oss << "  Выборочная дисперсия:    " << var << "\n";
        oss << "  Теоретическая дисперсия: " << theo_var << "\n\n";

        oss << "КРИТЕРИЙ χ² ПИРСОНА:\n";
        oss << "  Статистика χ²:           " << chi2_result.statistic << "\n";
        oss << "  Критическое значение:    " << chi2_result.critical_value << "\n";
        oss << "  P-value:                 " << chi2_result.p_value << "\n";
        oss << "  Степени свободы:         " << chi2_result.degrees_of_freedom << "\n";
        oss << "  Гипотеза H₀:             " << (chi2_result.reject_null ? "ОТВЕРГНУТА ❌" : "НЕ ОТВЕРГНУТА ✓") << "\n";

        output_buffer2->text(oss.str().c_str());

    } catch (const std::exception& e) {
        fl_alert("Ошибка: %s", e.what());
    }
}

int main() {
    Fl::set_font(FL_HELVETICA, "Arial");
    FL_NORMAL_SIZE = 13;

    Fl_Double_Window* window = new Fl_Double_Window(1100, 750, "Лабораторная работа №2");

    Fl_Tabs* tabs = new Fl_Tabs(10, 10, 1080, 730);

    {
        Fl_Group* task1 = new Fl_Group(10, 35, 1080, 705, "Задание 1: Непрерывное распределение");

        int x_offset = 25;
        int y_offset = 50;
        int label_w = 200;
        int input_w = 120;
        int line_h = 32;
        int spacing = 4;

        Fl_Box* title1 = new Fl_Box(x_offset, y_offset, 500, 25, "f(x) = λ·exp(-λx),  x ≥ 0");
        title1->labelfont(FL_HELVETICA_BOLD);
        title1->labelsize(15);
        title1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        y_offset += 35;

        Fl_Box* label_lambda = new Fl_Box(x_offset, y_offset, label_w, line_h, "Параметр λ:");
        label_lambda->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_lambda = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
        input_lambda->value("1.0");
        input_lambda->textsize(13);
        y_offset += line_h + spacing;

        Fl_Box* label_n1 = new Fl_Box(x_offset, y_offset, label_w, line_h, "Объём выборки n:");
        label_n1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_n1 = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
        input_n1->value("1000");
        input_n1->textsize(13);
        y_offset += line_h + spacing;

        Fl_Box* label_k = new Fl_Box(x_offset, y_offset, label_w, line_h, "Число интервалов k:");
        label_k->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_k = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
        input_k->textsize(13);
        y_offset += line_h + spacing;

        Fl_Box* hint_k = new Fl_Box(x_offset, y_offset, 350, 20, "(если пусто - используется формула Стёрджесса)");
        hint_k->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        hint_k->labelsize(11);
        hint_k->labelcolor(fl_rgb_color(0, 120, 0));
        y_offset += 25;

        Fl_Box* label_alpha1 = new Fl_Box(x_offset, y_offset, label_w, line_h, "Уровень значимости α:");
        label_alpha1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_alpha1 = new Fl_Input(x_offset + label_w, y_offset, input_w, line_h);
        input_alpha1->value("0.05");
        input_alpha1->textsize(13);
        y_offset += line_h + spacing * 4;

        Fl_Button* btn1 = new Fl_Button(x_offset, y_offset, 220, 40, "Сгенерировать");
        btn1->callback(task1_generate_callback);
        btn1->labelsize(13);

        histogram1 = new HistogramWidget(400, 50, 670, 280);

        output_buffer1 = new Fl_Text_Buffer();
        output_display1 = new Fl_Text_Display(400, 345, 670, 375);
        output_display1->buffer(output_buffer1);
        output_display1->textfont(FL_COURIER);
        output_display1->textsize(12);

        task1->end();
    }

    {
        Fl_Group* task2 = new Fl_Group(10, 35, 1080, 705, "Задание 2: Дискретное распределение");

        int x_offset = 25;
        int y_offset = 50;
        int label_w = 250;
        int input_w = 400;
        int line_h = 32;
        int spacing = 4;

        Fl_Box* title2 = new Fl_Box(x_offset, y_offset, 500, 25, "Дискретная случайная величина X");
        title2->labelfont(FL_HELVETICA_BOLD);
        title2->labelsize(15);
        title2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        y_offset += 35;

        Fl_Box* label_dist = new Fl_Box(x_offset, y_offset, 600, 22, "Распределение (формат: значение:вероятность, через запятую):");
        label_dist->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        label_dist->labelsize(12);
        y_offset += 26;

        input_distribution = new Fl_Multiline_Input(x_offset, y_offset, input_w, 75);
        input_distribution->value("1:0.1, 2:0.2, 3:0.4, 4:0.2, 5:0.1");
        input_distribution->textsize(13);
        y_offset += 80 + spacing * 2;

        Fl_Box* label_n2 = new Fl_Box(x_offset, y_offset, label_w, line_h, "Объём выборки n:");
        label_n2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_n2 = new Fl_Input(x_offset + label_w, y_offset, 120, line_h);
        input_n2->value("1000");
        input_n2->textsize(13);
        y_offset += line_h + spacing;

        Fl_Box* label_alpha2 = new Fl_Box(x_offset, y_offset, label_w, line_h, "Уровень значимости α:");
        label_alpha2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        input_alpha2 = new Fl_Input(x_offset + label_w, y_offset, 120, line_h);
        input_alpha2->value("0.05");
        input_alpha2->textsize(13);
        y_offset += line_h + spacing * 4;

        Fl_Button* btn2 = new Fl_Button(x_offset, y_offset, 220, 40, "Сгенерировать");
        btn2->callback(task2_generate_callback);
        btn2->labelsize(13);

        histogram2 = new HistogramWidget(450, 50, 620, 280);

        output_buffer2 = new Fl_Text_Buffer();
        output_display2 = new Fl_Text_Display(450, 345, 620, 375);
        output_display2->buffer(output_buffer2);
        output_display2->textfont(FL_COURIER);
        output_display2->textsize(12);

        task2->end();
    }

    tabs->end();

    window->end();
    window->show();

    return Fl::run();
}
