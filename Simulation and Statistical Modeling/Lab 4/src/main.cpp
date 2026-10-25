#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include "state_graph_widget.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct ModelResult {
    std::vector<double> probabilities;
    double mu = 0.0;
    double average_repaired = 0.0;
    double average_faulty = 0.0;
    double average_waiting = 0.0;
    double busy_probability = 0.0;
};

struct UiState {
    Fl_Input* machine_count = nullptr;
    Fl_Input* failure_rate = nullptr;
    Fl_Input* service_time = nullptr;
    StateGraphWidget* graph = nullptr;
    Fl_Text_Buffer* output_buffer = nullptr;
};

UiState ui;

double parse_positive(const Fl_Input* input, const char* name) {
    const std::string text = input->value();
    if (text.empty()) {
        throw std::invalid_argument(std::string("Введите ") + name + ".");
    }
    size_t processed = 0;
    const double value = std::stod(text, &processed);
    if (processed != text.size() || !std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(name) + " должно быть положительным числом.");
    }
    return value;
}

int parse_machine_count(const Fl_Input* input) {
    const std::string text = input->value();
    if (text.empty()) {
        throw std::invalid_argument("Введите число станков n.");
    }
    size_t processed = 0;
    const long long value = std::stoll(text, &processed);
    if (processed != text.size() || value < 1 || value > 1000) {
        throw std::invalid_argument("Число станков n должно быть целым числом от 1 до 1000.");
    }
    return static_cast<int>(value);
}

ModelResult calculate_model(int machine_count, double failure_rate, double service_time) {
    ModelResult result;
    result.mu = 1.0 / service_time;
    result.probabilities.assign(static_cast<size_t>(machine_count) + 1, 0.0);
    std::vector<double> log_weights(static_cast<size_t>(machine_count) + 1, 0.0);

    // Birth-death recurrence: b_k=(n-k)lambda, d_k=k*mu.
    for (int state = 1; state <= machine_count; ++state) {
        const double birth_rate = static_cast<double>(machine_count - state + 1) * failure_rate;
        const double death_rate = static_cast<double>(state) * result.mu;
        log_weights[state] = log_weights[state - 1] + std::log(birth_rate) - std::log(death_rate);
    }

    const double max_log_weight = *std::max_element(log_weights.begin(), log_weights.end());
    for (size_t state = 0; state < log_weights.size(); ++state) {
        result.probabilities[state] = std::exp(log_weights[state] - max_log_weight);
    }

    const double normalizer = std::accumulate(result.probabilities.begin(), result.probabilities.end(), 0.0);
    if (!std::isfinite(normalizer) || normalizer <= 0.0) {
        throw std::runtime_error("Не удалось нормировать вероятности для заданных параметров.");
    }
    for (double& probability : result.probabilities) {
        probability /= normalizer;
    }

    for (size_t state = 0; state < result.probabilities.size(); ++state) {
        const double probability = result.probabilities[state];
        result.average_faulty += static_cast<double>(state) * probability;
        if (state > 0) {
            result.average_repaired += result.mu * probability;
        }
        if (state > 1) {
            result.average_waiting += static_cast<double>(state - 1) * probability;
        }
    }
    result.busy_probability = 1.0 - result.probabilities[0];
    return result;
}

std::string make_report(int machine_count, double failure_rate, double service_time, const ModelResult& result) {
    std::ostringstream report;
    report << std::fixed << std::setprecision(6);
    report << "ВАРИАНТ 17. СМО «СТАНКИ — ОПЕРАТОР»\n";
    report << "=============================================\n";
    report << "Модель: состояние k означает, что неисправны k станков.\n";
    report << "Переходы: k -> k+1 с интенсивностью (n-k)lambda; "
            << "k -> k-1 с интенсивностью k*mu.\n\n";

    report << "Параметры\n";
    report << "  Число станков n:                 " << machine_count << "\n";
    report << "  Интенсивность отказа одного станка lambda: " << failure_rate << "  1/ч\n";
    report << "  Среднее время ремонта tобсл:       " << service_time << "  ч\n";
    report << "  Интенсивность ремонта mu:          " << result.mu << "  1/ч\n\n";

    report << "Финальные вероятности состояний\n";
    report << "  Состояние k        p_k\n";
    report << "  ------------------------\n";
    for (size_t state = 0; state < result.probabilities.size(); ++state) {
        report << "  " << std::setw(9) << state << "        " << result.probabilities[state] << "\n";
    }

    report << "\nХарактеристики эффективности\n";
    report << "  A   — среднее число ремонтов в час: " << result.average_repaired << "\n";
    report << "  w   — среднее число неисправных станков: " << result.average_faulty << "\n";
    report << "  R   — среднее число станков в очереди: " << result.average_waiting << "\n";
    report << "  Pзан — вероятность занятости оператора: " << result.busy_probability << "\n";
    report << "\nПроверка: сумма вероятностей = "
            << std::accumulate(result.probabilities.begin(), result.probabilities.end(), 0.0) << "\n";
    return report.str();
}

void calculate_callback(Fl_Widget*, void*) {
    try {
        const int machine_count = parse_machine_count(ui.machine_count);
        const double failure_rate = parse_positive(ui.failure_rate, "lambda");
        const double service_time = parse_positive(ui.service_time, "среднее время ремонта");
        const ModelResult result = calculate_model(machine_count, failure_rate, service_time);

        ui.graph->set_probabilities(result.probabilities);
        const std::string report = make_report(machine_count, failure_rate, service_time, result);
        ui.output_buffer->text(report.c_str());
    } catch (const std::exception& error) {
        fl_alert("Ошибка ввода или расчёта: %s", error.what());
    }
}

void clear_callback(Fl_Widget*, void*) {
    ui.graph->clear();
    ui.output_buffer->text("Введите параметры модели и нажмите «Рассчитать».\n");
}

Fl_Box* make_label(int x, int y, int width, int height, const char* text) {
    auto* label = new Fl_Box(x, y, width, height, text);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(13);
    return label;
}
}

int main() {
    Fl::set_font(FL_HELVETICA, "Arial");
    FL_NORMAL_SIZE = 13;

    Fl_Double_Window window(1240, 820, "Лабораторная работа №4 — СМО станков и ремонтника");
    window.color(fl_rgb_color(245, 247, 249));

    Fl_Group controls(20, 20, 300, 770);
    controls.box(FL_BORDER_BOX);
    controls.color(FL_WHITE);

    auto* title = new Fl_Box(38, 40, 260, 30, "Параметры модели");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    title->labelfont(FL_HELVETICA_BOLD);
    title->labelsize(18);
    title->labelcolor(fl_rgb_color(42, 77, 105));

    auto* subtitle = new Fl_Box(38, 78, 260, 54,
        "Один оператор ремонтирует\nнеисправные станки в порядке очереди.");
    subtitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    subtitle->labelsize(12);
    subtitle->labelcolor(fl_rgb_color(80, 80, 80));

    make_label(38, 158, 230, 26, "Количество станков n:");
    ui.machine_count = new Fl_Input(38, 184, 240, 32);
    ui.machine_count->type(FL_INT_INPUT);
    ui.machine_count->value("8");

    make_label(38, 232, 230, 26, "Интенсивность отказа lambda (1/ч):");
    ui.failure_rate = new Fl_Input(38, 258, 240, 32);
    ui.failure_rate->type(FL_FLOAT_INPUT);
    ui.failure_rate->value("0.2");

    make_label(38, 306, 240, 26, "Среднее время ремонта tобсл (ч):");
    ui.service_time = new Fl_Input(38, 332, 240, 32);
    ui.service_time->type(FL_FLOAT_INPUT);
    ui.service_time->value("1");

    auto* hint = new Fl_Box(38, 382, 240, 80,
        "Для каждого станка поток отказов\nпростейший с интенсивностью lambda.\nВремя ремонта показательное.");
    hint->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    hint->labelsize(12);
    hint->labelcolor(fl_rgb_color(90, 90, 90));

    auto* calculate = new Fl_Button(38, 500, 240, 42, "Рассчитать");
    calculate->labelsize(14);
    calculate->callback(calculate_callback);

    auto* clear = new Fl_Button(38, 554, 240, 34, "Очистить результаты");
    clear->callback(clear_callback);

    auto* legend = new Fl_Box(38, 635, 240, 110,
        "Обозначения:\n"
        "A — средняя производительность\n"
        "w — неисправные станки\n"
        "R — очередь на ремонт\n"
        "Pзан — занятость оператора");
    legend->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    legend->labelsize(12);
    legend->labelcolor(fl_rgb_color(70, 70, 70));
    controls.end();

    ui.graph = new StateGraphWidget(340, 20, 880, 255);
    ui.output_buffer = new Fl_Text_Buffer();
    auto* output = new Fl_Text_Display(340, 295, 880, 495);
    output->buffer(ui.output_buffer);
    output->textfont(FL_COURIER);
    output->textsize(13);
    output->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    ui.output_buffer->text("Введите параметры модели и нажмите «Рассчитать».\n");

    window.end();
    window.show();
    return Fl::run();
}
