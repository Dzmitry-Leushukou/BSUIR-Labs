#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include "state_graph_widget.hpp"
#include "repair_model.hpp"
#include "ui_scale.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct UiState {
    Fl_Input* machine_count = nullptr;
    Fl_Input* failure_rate = nullptr;
    Fl_Input* service_time = nullptr;
    Fl_Input* observation_time = nullptr;
    Fl_Input* seed = nullptr;
    Fl_Button* calculate = nullptr;
    Fl_Button* simulate = nullptr;
    Fl_Button* stop = nullptr;
    Fl_Progress* progress = nullptr;
    Fl_Box* status = nullptr;
    StateGraphWidget* graph = nullptr;
    Fl_Text_Buffer* output_buffer = nullptr;
    Fl_Text_Display* output = nullptr;
    std::unique_ptr<RepairSimulation> simulation;
    ModelParameters model;
    SimulationParameters experiment;
    ModelResult theory;
};

UiState ui;

double parse_number(const Fl_Input* input, const char* name) {
    const std::string text = input->value();
    if (text.empty()) {
        throw std::invalid_argument(std::string("Введите ") + name + ".");
    }
    size_t processed = 0;
    const double value = std::stod(text, &processed);
    if (processed != text.size() || !std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(name) + " должно быть положительным конечным числом.");
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

ModelParameters read_model() {
    return {parse_machine_count(ui.machine_count),
        parse_number(ui.failure_rate, "lambda"), parse_number(ui.service_time, "Среднее время ремонта")};
}

SimulationParameters read_experiment() {
    const std::string seed = ui.seed->value();
    if (seed.empty() || seed.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Зерно должно быть целым неотрицательным числом.");
    }
    return {parse_number(ui.observation_time, "Время наблюдения"),
        std::stoull(seed)};
}

void comparison_row(std::ostringstream& stream, const std::string& name,
                    double theory, double experiment) {
    const double error = std::abs(experiment - theory);
    stream << "  " << std::left << std::setw(8) << name << std::right
           << std::setw(13) << theory << std::setw(13) << experiment << std::setw(13) << error;
    if (theory > 0.0 && std::isfinite(100.0 * error / theory)) {
        stream << std::setw(13) << 100.0 * error / theory;
    } else {
        stream << std::setw(13) << "--";
    }
    stream << '\n';
}

std::string make_report(const ModelParameters& model, const ModelResult& theory,
                        const SimulationResult* simulation = nullptr) {
    std::ostringstream report;
    report << std::fixed << std::setprecision(6);
    report << "ВАРИАНТ 17. СМО «СТАНКИ — ОПЕРАТОР»\n";
    report << "=============================================================\n";
    report << "Модель: состояние k означает, что неисправны k станков.\n";
    report << "Переходы: k -> k+1 с интенсивностью (n-k)lambda; "
            << "k -> k-1 с интенсивностью mu при k > 0.\n\n";
    report << "n = " << model.machine_count << "; lambda = " << model.failure_rate << " 1/ч; "
           << "tобсл = " << model.service_time << " ч; mu = " << theory.mu << " 1/ч.\n\n";
    if (simulation) {
        report << "ИМИТАЦИЯ: теория и эксперимент\n"
               << "  Наблюдение T = " << ui.experiment.observation_time << " ч; зерно = " << ui.experiment.seed << ".\n"
               << "  Отказы: " << simulation->failures << "; завершённые ремонты: " << simulation->repairs << ".\n"
               << "  Всего событий: " << simulation->processed_events << ". Начальное состояние: все исправны.\n"
               << "  Статистика собирается с момента запуска за всё время T.\n\n"
               << "Показатели       Теория     Имитация     |Разность|    Ошибка, %\n";
        const auto& empirical = simulation->estimates;
        comparison_row(report, "A", theory.average_repaired, empirical.average_repaired);
        comparison_row(report, "w", theory.average_faulty, empirical.average_faulty);
        comparison_row(report, "R", theory.average_waiting, empirical.average_waiting);
        comparison_row(report, "P_zan", theory.busy_probability, empirical.busy_probability);
        report << "\nВероятности: p = время в состоянии / T\n"
               << "  Состояние       Теория     Имитация     |Разность|    Ошибка, %\n";
        double max_error = 0.0;
        for (std::size_t k = 0; k < theory.probabilities.size(); ++k) {
            comparison_row(report, "S" + std::to_string(k), theory.probabilities[k], empirical.probabilities[k]);
            max_error = std::max(max_error, std::abs(theory.probabilities[k] - empirical.probabilities[k]));
        }
        report << "\nМаксимальная абсолютная разность вероятностей: " << max_error << "\n"
               << "Сумма вероятностей: теория = "
               << std::accumulate(theory.probabilities.begin(), theory.probabilities.end(), 0.0)
               << "; имитация = " << std::accumulate(empirical.probabilities.begin(), empirical.probabilities.end(), 0.0) << "\n"
               << "Баланс числа неисправных за наблюдение: " << simulation->initial_state << " + "
               << simulation->failures << " - " << simulation->repairs << " = " << simulation->final_state << "\n"
               << "\nA = число завершённых ремонтов / T, единица: 1/ч.\n"
               << "w — среднее число неисправных; R — средняя очередь; P_zan — занятость.\n"
               << "Увеличьте T для более устойчивой оценки; смените зерно для другого опыта.\n"
               << "Одинаковое зерно воспроизводит тот же прогон при тех же параметрах.\n";
        if (simulation->failures + simulation->repairs < 1000) {
            report << "\nНаблюдений мало: увеличьте T, чтобы сравнение было содержательнее.\n";
        }
    } else {
        report << "ТЕОРИЯ: стационарные показатели\n"
               << "  A — ремонты в час: " << theory.average_repaired << "\n"
               << "  w — среднее число неисправных: " << theory.average_faulty << "\n"
               << "  R — средняя очередь: " << theory.average_waiting << "\n"
               << "  Pзан — занятость оператора: " << theory.busy_probability << "\n\n"
               << "  Состояние        Вероятность\n";
        for (std::size_t k = 0; k < theory.probabilities.size(); ++k) {
            report << "  " << std::setw(9) << k << std::setw(19) << theory.probabilities[k] << '\n';
        }
        report << "\nНажмите «Запустить имитацию» для сравнения с экспериментом.\n";
    }
    return report.str();
}

void show_report(const std::string& report) {
    ui.output_buffer->text(report.c_str());
    ui.output->insert_position(0);
    ui.output->scroll(0, 0);
}

void set_running(bool running) {
    Fl_Widget* fields[] = {ui.machine_count, ui.failure_rate, ui.service_time,
        ui.observation_time, ui.seed, ui.calculate, ui.simulate};
    for (auto* field : fields) {
        if (running) field->deactivate(); else field->activate();
    }
    if (running) ui.stop->activate(); else ui.stop->deactivate();
}

void simulation_tick(void*);

void stop_callback(Fl_Widget*, void*) {
    Fl::remove_timeout(simulation_tick);
    if (ui.simulation) {
        ui.simulation.reset();
        ui.status->copy_label("Имитация остановлена. Показана только теория.");
        ui.progress->copy_label("Остановлено");
    }
    set_running(false);
}

void simulation_tick(void*) {
    if (!ui.simulation) return;
    try {
        const bool finished = ui.simulation->advance();
        ui.progress->value(static_cast<float>(ui.simulation->progress()));
        if (finished) {
            const auto result = ui.simulation->result();
            show_report(make_report(ui.model, ui.theory, &result));
            ui.graph->set_probabilities(ui.theory.probabilities, result.estimates.probabilities);
            ui.simulation.reset();
            set_running(false);
            ui.progress->copy_label("Готово");
            ui.status->copy_label("Сравнение готово: теория, имитация и отклонения — в таблице ниже.");
        } else {
            std::ostringstream status;
            status << "Наблюдение: " << std::fixed << std::setprecision(1)
                   << 100.0 * ui.simulation->progress() << "%; событий: " << ui.simulation->processed_events();
            ui.status->copy_label(status.str().c_str());
            Fl::repeat_timeout(0.01, simulation_tick);
        }
    } catch (const std::exception& error) {
        stop_callback(nullptr, nullptr);
        ui.status->copy_label("Имитация не завершена. Проверьте параметры.");
        fl_alert("Ошибка имитации: %s", error.what());
    }
}

void calculate_callback(Fl_Widget*, void*) {
    try {
        const auto model = read_model();
        const auto result = calculate_model(model);
        ui.graph->set_probabilities(result.probabilities);
        show_report(make_report(model, result));
        ui.progress->value(0.0f);
        ui.progress->copy_label("Теория");
        ui.status->copy_label("Теория рассчитана. Для эксперимента задайте T и зерно.");
    } catch (const std::exception& error) {
        fl_alert("Ошибка ввода или расчёта: %s", error.what());
    }
}

void simulate_callback(Fl_Widget*, void*) {
    try {
        const auto model = read_model();
        const auto experiment = read_experiment();
        const auto theory = calculate_model(model);
        auto simulation = std::make_unique<RepairSimulation>(model, experiment);
        ui.model = model;
        ui.experiment = experiment;
        ui.theory = theory;
        ui.simulation = std::move(simulation);
        ui.graph->set_probabilities(theory.probabilities);
        show_report(make_report(model, theory));
        ui.progress->value(0.0f);
        ui.progress->copy_label("Идёт имитация");
        ui.status->copy_label("Генерируются случайные отказы станков и ремонты. Можно остановить прогон.");
        set_running(true);
        Fl::add_timeout(0.01, simulation_tick);
    } catch (const std::exception& error) {
        fl_alert("Ошибка ввода или расчёта: %s", error.what());
    }
}

void clear_callback(Fl_Widget*, void*) {
    stop_callback(nullptr, nullptr);
    ui.graph->clear();
    show_report("Введите параметры модели и запустите теоретический расчёт или имитацию.\n");
    ui.progress->value(0.0f);
    ui.progress->copy_label("Готово к запуску");
    ui.status->copy_label("Статистика собирается с запуска. Время T задаётся в модельных часах.");
}

Fl_Box* make_label(int x, int y, int width, int height, const char* text) {
    auto* label = new Fl_Box(x, y, width, height, text);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(ui_scale::px(15));
    return label;
}

Fl_Input* make_input(int label_y, const char* label, const char* value, const char* tooltip, bool integer = false) {
    make_label(ui_scale::px(38), ui_scale::px(label_y), ui_scale::px(270), ui_scale::px(26), label);
    auto* input = new Fl_Input(ui_scale::px(38), ui_scale::px(label_y + 27), ui_scale::px(264), ui_scale::px(34));
    input->type(integer ? FL_INT_INPUT : FL_FLOAT_INPUT);
    input->value(value);
    input->textsize(ui_scale::px(16));
    input->tooltip(tooltip);
    return input;
}
}

int main() {
#ifdef _WIN32
    SetProcessDPIAware();
#endif

    float dpi_x = 96.0f;
    float dpi_y = 96.0f;
    Fl::screen_dpi(dpi_x, dpi_y);

    int screen_x = 0;
    int screen_y = 0;
    int screen_w = 0;
    int screen_h = 0;
    Fl::screen_work_area(screen_x, screen_y, screen_w, screen_h, 0);
    const float dpi_scale = dpi_y > 0.0f ? dpi_y / 96.0f : 1.0f;
    const float fit_scale = std::min(
        screen_w > 0 ? static_cast<float>(screen_w) / 1240.0f : dpi_scale,
        screen_h > 0 ? static_cast<float>(screen_h) / 820.0f : dpi_scale);
    ui_scale::factor = std::max(1.0f, std::min(dpi_scale, fit_scale));

    Fl::set_font(FL_HELVETICA, "Segoe UI");
    Fl::set_font(FL_COURIER, "Consolas");
    FL_NORMAL_SIZE = ui_scale::px(15);

    Fl_Double_Window window(ui_scale::px(1240), ui_scale::px(820), "Лабораторная работа №4 — СМО станков и ремонтника");
    window.color(fl_rgb_color(245, 247, 249));

    Fl_Group controls(ui_scale::px(20), ui_scale::px(20), ui_scale::px(310), ui_scale::px(780));
    controls.box(FL_BORDER_BOX);
    controls.color(FL_WHITE);

    auto* title = new Fl_Box(ui_scale::px(38), ui_scale::px(40), ui_scale::px(260), ui_scale::px(32), "Параметры модели");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    title->labelfont(FL_HELVETICA_BOLD);
    title->labelsize(ui_scale::px(20));
    title->labelcolor(fl_rgb_color(42, 77, 105));

    auto* subtitle = new Fl_Box(ui_scale::px(38), ui_scale::px(78), ui_scale::px(270), ui_scale::px(45),
        "Один оператор, n станков.\nТеория и случайный эксперимент.");
    subtitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    subtitle->labelsize(ui_scale::px(14));
    subtitle->labelcolor(fl_rgb_color(80, 80, 80));

    ui.machine_count = make_input(130, "Количество станков n:", "8", "Целое число от 1 до 1000.", true);
    ui.failure_rate = make_input(200, "Отказы одного станка lambda (1/ч):", "0.2", "lambda: интенсивность отказа одного работающего станка.");
    ui.service_time = make_input(270, "Среднее время ремонта (ч):", "1", "mu = 1 / среднее время ремонта. Один оператор чинит один станок.");
    ui.observation_time = make_input(340, "Время наблюдения T (ч):", "100000", "Модельное время с момента запуска. Длиннее наблюдение — устойчивее оценки.");
    ui.seed = make_input(410, "Зерно случайных чисел:", "42", "Одинаковое зерно повторяет опыт. Другое зерно — другая случайная последовательность.", true);

    auto* hint = new Fl_Box(ui_scale::px(38), ui_scale::px(479), ui_scale::px(270), ui_scale::px(45),
        "T — модельные часы.\nВ начале все станки исправны.");
    hint->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    hint->labelsize(ui_scale::px(13));
    hint->labelcolor(fl_rgb_color(90, 90, 90));

    ui.calculate = new Fl_Button(ui_scale::px(38), ui_scale::px(533), ui_scale::px(264), ui_scale::px(36), "Рассчитать теорию");
    ui.calculate->labelsize(ui_scale::px(15));
    ui.calculate->callback(calculate_callback);
    ui.simulate = new Fl_Button(ui_scale::px(38), ui_scale::px(579), ui_scale::px(264), ui_scale::px(40), "Запустить имитацию");
    ui.simulate->labelsize(ui_scale::px(16));
    ui.simulate->callback(simulate_callback);
    ui.stop = new Fl_Button(ui_scale::px(38), ui_scale::px(629), ui_scale::px(124), ui_scale::px(34), "Остановить");
    ui.stop->labelsize(ui_scale::px(14));
    ui.stop->callback(stop_callback);
    ui.stop->deactivate();
    auto* clear = new Fl_Button(ui_scale::px(172), ui_scale::px(629), ui_scale::px(130), ui_scale::px(34), "Очистить");
    clear->labelsize(ui_scale::px(14));
    clear->callback(clear_callback);

    auto* legend = new Fl_Box(ui_scale::px(38), ui_scale::px(673), ui_scale::px(270), ui_scale::px(43),
        "A — ремонты/ч; w — неисправные;\nR — очередь; Pзан — занятость.");
    legend->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    legend->labelsize(ui_scale::px(13));
    legend->labelcolor(fl_rgb_color(70, 70, 70));
    controls.end();

    ui.graph = new StateGraphWidget(ui_scale::px(350), ui_scale::px(20), ui_scale::px(870), ui_scale::px(290));
    ui.progress = new Fl_Progress(ui_scale::px(350), ui_scale::px(320), ui_scale::px(870), ui_scale::px(24));
    ui.progress->minimum(0.0f);
    ui.progress->maximum(1.0f);
    ui.progress->selection_color(fl_rgb_color(164, 213, 204));
    ui.progress->labelsize(ui_scale::px(13));
    ui.progress->copy_label("Готово к запуску");
    ui.status = new Fl_Box(ui_scale::px(350), ui_scale::px(349), ui_scale::px(870), ui_scale::px(25),
        "Статистика собирается с запуска. Время T задаётся в модельных часах.");
    ui.status->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    ui.status->labelsize(ui_scale::px(13));
    ui.output_buffer = new Fl_Text_Buffer();
    auto* output = new Fl_Text_Display(ui_scale::px(350), ui_scale::px(382), ui_scale::px(870), ui_scale::px(418));
    ui.output = output;
    output->buffer(ui.output_buffer);
    output->textfont(FL_COURIER);
    output->textsize(ui_scale::px(14));
    output->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
    ui.output_buffer->text("Введите параметры модели.\n«Рассчитать теорию» — расчёт по формулам.\n«Запустить имитацию» — случайные отказы и ремонты, сравнение с теорией.\n");

    window.end();
    window.show();
    const int exit_code = Fl::run();
    Fl::remove_timeout(simulation_tick);
    ui.simulation.reset();
    return exit_code;
}
