#include "histogram_widget.hpp"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

HistogramWidget::HistogramWidget(int X, int Y, int W, int H, const char* L)
    : Fl_Widget(X, Y, W, H, L), type_(PlotType::Histogram) {
}

void HistogramWidget::set_histogram_data(
    const std::vector<double>& bin_edges,
    const std::vector<double>& heights,
    std::function<double(double)> density_func
) {
    type_ = PlotType::Histogram;
    bin_edges_ = bin_edges;
    heights_ = heights;
    density_func_ = density_func;
    redraw();
}

void HistogramWidget::set_bar_chart_data(
    const std::vector<int>& values,
    const std::vector<double>& frequencies,
    const std::vector<double>& theoretical_probs
) {
    type_ = PlotType::BarChart;
    values_ = values;
    frequencies_ = frequencies;
    theoretical_probs_ = theoretical_probs;
    redraw();
}

void HistogramWidget::draw() {
    // Фон
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());

    // Рамка
    fl_color(FL_BLACK);
    fl_rect(x(), y(), w(), h());

    if (type_ == PlotType::Histogram) {
        draw_histogram();
    } else {
        draw_bar_chart();
    }
}

void HistogramWidget::draw_histogram() {
    if (bin_edges_.empty() || heights_.empty()) return;

    const int left_margin = 110;
    const int other_margin = 50;
    const int plot_x = x() + left_margin;
    const int plot_y = y() + other_margin;
    const int plot_w = w() - left_margin - other_margin;
    const int plot_h = h() - 2 * other_margin;

    double x_min = bin_edges_.front();
    double x_max = bin_edges_.back();
    double y_max = *std::max_element(heights_.begin(), heights_.end());

    // Добавляем запас для кривой плотности
    if (density_func_) {
        for (size_t i = 0; i < bin_edges_.size(); ++i) {
            y_max = std::max(y_max, density_func_(bin_edges_[i]));
        }
    }
    y_max *= 1.1;

    draw_axes(x_min, x_max, 0.0, y_max);

    // Отрисовка столбцов гистограммы
    fl_color(fl_rgb_color(100, 150, 255));
    for (size_t i = 0; i < heights_.size(); ++i) {
        double x1 = bin_edges_[i];
        double x2 = bin_edges_[i + 1];

        int sx1 = plot_x + static_cast<int>((x1 - x_min) / (x_max - x_min) * plot_w);
        int sx2 = plot_x + static_cast<int>((x2 - x_min) / (x_max - x_min) * plot_w);
        int sy = plot_y + plot_h - static_cast<int>(heights_[i] / y_max * plot_h);

        fl_rectf(sx1, sy, sx2 - sx1, plot_y + plot_h - sy);
        fl_color(FL_BLACK);
        fl_rect(sx1, sy, sx2 - sx1, plot_y + plot_h - sy);
        fl_color(fl_rgb_color(100, 150, 255));
    }

    // Отрисовка теоретической кривой плотности
    if (density_func_) {
        fl_color(FL_RED);
        fl_line_style(FL_SOLID, 2);

        int prev_sx = -1, prev_sy = -1;
        for (int i = 0; i <= plot_w; ++i) {
            double x_val = x_min + (x_max - x_min) * i / plot_w;
            double y_val = density_func_(x_val);

            int sx = plot_x + i;
            int sy = plot_y + plot_h - static_cast<int>(y_val / y_max * plot_h);

            if (prev_sx >= 0) {
                fl_line(prev_sx, prev_sy, sx, sy);
            }

            prev_sx = sx;
            prev_sy = sy;
        }

        fl_line_style(FL_SOLID, 1);
    }
}

void HistogramWidget::draw_bar_chart() {
    if (values_.empty() || frequencies_.empty()) return;

    const int margin = 50;
    const int plot_x = x() + margin;
    const int plot_y = y() + margin;
    const int plot_w = w() - 2 * margin;
    const int plot_h = h() - 2 * margin;

    int x_min = *std::min_element(values_.begin(), values_.end());
    int x_max = *std::max_element(values_.begin(), values_.end());
    double y_max = *std::max_element(frequencies_.begin(), frequencies_.end());

    if (!theoretical_probs_.empty()) {
        y_max = std::max(y_max, *std::max_element(theoretical_probs_.begin(), theoretical_probs_.end()));
    }
    y_max *= 1.1;

    draw_axes(x_min, x_max, 0.0, y_max);

    // Отрисовка столбцов
    fl_color(fl_rgb_color(100, 200, 100));
    int bar_width = plot_w / (values_.size() * 2);

    for (size_t i = 0; i < values_.size(); ++i) {
        int val = values_[i];
        double freq = frequencies_[i];

        int sx = plot_x + static_cast<int>((val - x_min) / static_cast<double>(x_max - x_min + 1) * plot_w) - bar_width / 2;
        int sy = plot_y + plot_h - static_cast<int>(freq / y_max * plot_h);
        int sh = plot_y + plot_h - sy;

        fl_rectf(sx, sy, bar_width, sh);
        fl_color(FL_BLACK);
        fl_rect(sx, sy, bar_width, sh);
        fl_color(fl_rgb_color(100, 200, 100));
    }

    // Отрисовка теоретических вероятностей (маркеры)
    if (!theoretical_probs_.empty() && theoretical_probs_.size() == values_.size()) {
        fl_color(FL_RED);
        for (size_t i = 0; i < values_.size(); ++i) {
            int val = values_[i];
            double prob = theoretical_probs_[i];

            int sx = plot_x + static_cast<int>((val - x_min) / static_cast<double>(x_max - x_min + 1) * plot_w);
            int sy = plot_y + plot_h - static_cast<int>(prob / y_max * plot_h);

            // Круг
            fl_circle(sx, sy, 4);
            // Горизонтальная линия
            fl_line(sx - 10, sy, sx + 10, sy);
        }
    }
}

void HistogramWidget::draw_axes(double x_min, double x_max, double y_min, double y_max) {
    const int left_margin = 110;
    const int other_margin = 50;
    const int plot_x = x() + left_margin;
    const int plot_y = y() + other_margin;
    const int plot_w = w() - left_margin - other_margin;
    const int plot_h = h() - 2 * other_margin;

    fl_color(FL_BLACK);
    fl_line_style(FL_SOLID, 1);

    // Оси
    fl_line(plot_x, plot_y + plot_h, plot_x + plot_w, plot_y + plot_h); // X
    fl_line(plot_x, plot_y, plot_x, plot_y + plot_h);                   // Y

    fl_font(FL_HELVETICA, 10);

    // Деления и подписи на оси X
    const int num_x_ticks = 5;
    for (int i = 0; i <= num_x_ticks; ++i) {
        double x_val = x_min + (x_max - x_min) * i / num_x_ticks;
        int sx = plot_x + plot_w * i / num_x_ticks;
        fl_line(sx, plot_y + plot_h, sx, plot_y + plot_h + 5);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << x_val;
        fl_draw(oss.str().c_str(), sx - 15, plot_y + plot_h + 20);
    }

    // Деления и подписи на оси Y
    const int num_y_ticks = 5;
    for (int i = 0; i <= num_y_ticks; ++i) {
        double y_val = y_min + (y_max - y_min) * i / num_y_ticks;
        int sy = plot_y + plot_h - plot_h * i / num_y_ticks;
        fl_line(plot_x - 5, sy, plot_x, sy);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << y_val;
        std::string label = oss.str();
        int label_width = static_cast<int>(label.length() * 7);
        fl_draw(label.c_str(), plot_x - label_width - 10, sy + 5);
    }
}
