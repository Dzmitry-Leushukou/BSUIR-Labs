#include "state_graph_widget.hpp"

#include <FL/fl_draw.H>

#include "ui_scale.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr int kMaxVisibleStates = 9;

std::string format_probability(double value) {
    if (value > 0.0 && value < 0.0005) {
        return "<0.001";
    }
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void draw_centered(const std::string& text, int center_x, int baseline_y) {
    fl_draw(text.c_str(), center_x - static_cast<int>(fl_width(text.c_str()) / 2.0), baseline_y);
}
}

StateGraphWidget::StateGraphWidget(int x, int y, int w, int h, const char* label)
    : Fl_Widget(x, y, w, h, label) {
    box(FL_BORDER_BOX);
}

void StateGraphWidget::set_probabilities(const std::vector<double>& probabilities,
                                         const std::vector<double>& empirical) {
    probabilities_ = probabilities;
    empirical_ = empirical;
    redraw();
}

void StateGraphWidget::clear() {
    probabilities_.clear();
    empirical_.clear();
    redraw();
}

void StateGraphWidget::draw_state(int state, int center_x, int center_y, int radius, double probability) const {
    fl_color(probability >= 0.05 ? fl_rgb_color(218, 235, 250) : FL_WHITE);
    fl_pie(center_x - radius, center_y - radius, 2 * radius, 2 * radius, 0.0, 360.0);
    fl_color(fl_rgb_color(38, 68, 91));
    fl_line_style(FL_SOLID, ui_scale::px(2));
    fl_circle(center_x, center_y, radius);
    fl_line_style(FL_SOLID, 1);

    fl_color(fl_rgb_color(26, 43, 56));
    fl_font(FL_HELVETICA_BOLD, ui_scale::px(17));
    std::ostringstream state_label;
    state_label << "S" << state;
    draw_centered(state_label.str(), center_x, center_y + ui_scale::px(6));

    fl_font(FL_HELVETICA, ui_scale::px(12));
    fl_color(fl_rgb_color(38, 68, 91));
    const std::string probability_text = format_probability(probability);
    draw_centered("Т: " + probability_text, center_x, center_y + ui_scale::px(80));
    if (empirical_.size() == probabilities_.size()) {
        fl_color(fl_rgb_color(159, 85, 26));
        draw_centered("И: " + format_probability(empirical_[state]), center_x, center_y + ui_scale::px(98));
    }
}

void StateGraphWidget::draw_arrow(int x1, int y1, int x2, int y2, bool upper) const {
    const int offset = upper ? -ui_scale::px(41) : ui_scale::px(41);
    const int line_y = y1 + offset;
    fl_color(fl_rgb_color(77, 91, 101));
    fl_line_style(FL_SOLID, ui_scale::px(2));
    fl_line(x1, line_y, x2, line_y);

    const int direction = x2 >= x1 ? 1 : -1;
    fl_line(x2, line_y, x2 - direction * ui_scale::px(7), line_y - ui_scale::px(4));
    fl_line(x2, line_y, x2 - direction * ui_scale::px(7), line_y + ui_scale::px(4));

    fl_line_style(FL_SOLID, 1);
    fl_color(fl_rgb_color(49, 65, 76));
    fl_font(FL_HELVETICA, ui_scale::px(12));
    const std::string rate = upper ? "(n-k) lambda" : "mu";
    draw_centered(rate, (x1 + x2) / 2, line_y + (upper ? -ui_scale::px(7) : ui_scale::px(20)));
}

void StateGraphWidget::draw() {
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());
    fl_color(fl_rgb_color(134, 146, 155));
    fl_rect(x(), y(), w(), h());

    fl_color(fl_rgb_color(38, 68, 91));
    fl_font(FL_HELVETICA_BOLD, ui_scale::px(17));
    fl_draw("Граф состояний", x() + ui_scale::px(16), y() + ui_scale::px(29));
    fl_font(FL_HELVETICA, ui_scale::px(12));
    fl_draw("Т — теория; И — имитация. Голубая заливка: теоретическая вероятность не ниже 5%.",
        x() + ui_scale::px(16), y() + ui_scale::px(52));

    if (probabilities_.empty()) {
        fl_color(fl_rgb_color(100, 100, 100));
        fl_font(FL_HELVETICA, ui_scale::px(15));
        fl_draw("Рассчитайте теорию или запустите имитацию", x() + ui_scale::px(28), y() + h() / 2);
        return;
    }

    std::vector<int> visible_states;
    const int state_count = static_cast<int>(probabilities_.size());
    if (state_count <= kMaxVisibleStates) {
        for (int state = 0; state < state_count; ++state) {
            visible_states.push_back(state);
        }
    } else {
        for (int state = 0; state < 4; ++state) {
            visible_states.push_back(state);
        }
        visible_states.push_back(-1);
        for (int state = state_count - 4; state < state_count; ++state) {
            visible_states.push_back(state);
        }
    }

    const int center_y = y() + h() / 2 - ui_scale::px(10);
    const int margin = ui_scale::px(58);
    const int usable_width = std::max(1, w() - 2 * margin);
    const int gap = visible_states.size() > 1
        ? usable_width / static_cast<int>(visible_states.size() - 1)
        : usable_width;
    const int radius = std::min(ui_scale::px(30), std::max(ui_scale::px(18), (gap - ui_scale::px(12)) / 2));

    std::vector<int> centers;
    centers.reserve(visible_states.size());
    for (size_t index = 0; index < visible_states.size(); ++index) {
        centers.push_back(x() + margin + static_cast<int>(index) * gap);
    }

    for (size_t index = 0; index + 1 < visible_states.size(); ++index) {
        if (visible_states[index] < 0 || visible_states[index + 1] < 0) {
            continue;
        }
        draw_arrow(centers[index] + radius, center_y, centers[index + 1] - radius, center_y, true);
        draw_arrow(centers[index + 1] - radius, center_y, centers[index] + radius, center_y, false);
    }

    for (size_t index = 0; index < visible_states.size(); ++index) {
        const int state = visible_states[index];
        if (state < 0) {
            fl_color(fl_rgb_color(75, 75, 75));
            fl_font(FL_HELVETICA_BOLD, ui_scale::px(22));
            draw_centered("...", centers[index], center_y + ui_scale::px(8));
            continue;
        }
        draw_state(state, centers[index], center_y, radius, probabilities_[state]);
    }

    fl_color(fl_rgb_color(90, 90, 90));
    fl_font(FL_HELVETICA, ui_scale::px(13));
    fl_draw("При k неисправных станках: отказ (n-k) lambda, ремонт mu при k > 0",
        x() + ui_scale::px(16), y() + h() - ui_scale::px(15));
}
