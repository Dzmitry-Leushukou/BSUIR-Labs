#pragma once

#include <FL/Fl_Widget.H>
#include <vector>
#include <functional>

enum class PlotType {
    Histogram,      // Гистограмма с наложенной кривой плотности
    BarChart        // Столбчатая диаграмма с маркерами
};

class HistogramWidget : public Fl_Widget {
public:
    HistogramWidget(int X, int Y, int W, int H, const char* L = nullptr);

    // Установка данных для гистограммы (непрерывная СВ)
    void set_histogram_data(
        const std::vector<double>& bin_edges,
        const std::vector<double>& heights,
        std::function<double(double)> density_func
    );

    // Установка данных для столбчатой диаграммы (дискретная СВ)
    void set_bar_chart_data(
        const std::vector<int>& values,
        const std::vector<double>& frequencies,
        const std::vector<double>& theoretical_probs
    );

    void draw() override;

private:
    PlotType type_;

    // Данные для гистограммы
    std::vector<double> bin_edges_;
    std::vector<double> heights_;
    std::function<double(double)> density_func_;

    // Данные для столбчатой диаграммы
    std::vector<int> values_;
    std::vector<double> frequencies_;
    std::vector<double> theoretical_probs_;

    void draw_histogram();
    void draw_bar_chart();
    void draw_axes(double x_min, double x_max, double y_min, double y_max);
};
