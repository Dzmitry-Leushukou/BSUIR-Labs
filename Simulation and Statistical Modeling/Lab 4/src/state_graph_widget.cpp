#include "state_graph_widget.hpp"

#include <FL/fl_draw.H>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>

namespace {
constexpr int kRadius = 27;
constexpr int kHorizontalMargin = 58;
constexpr int kMaxVisibleStates = 16;

std::string format_probability(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}
}

StateGraphWidget::StateGraphWidget(int x, int y, int w, int h, const char* label)
    : Fl_Widget(x, y, w, h, label) {
    box(FL_BORDER_BOX);
}

void StateGraphWidget::set_probabilities(const std::vector<double>& probabilities) {
    probabilities_ = probabilities;
    redraw();
}

void StateGraphWidget::clear() {
    probabilities_.clear();
    redraw();
}

void StateGraphWidget::draw_state(int state, int center_x, int center_y, int radius, double probability) const {
    const bool highlighted = probability > 0.0;
    fl_color(highlighted ? fl_rgb_color(222, 238, 255) : FL_WHITE);
    fl_pie(center_x - radius, center_y - radius, 2 * radius, 2 * radius, 0.0, 360.0);
    fl_color(fl_rgb_color(42, 77, 105));
    fl_circle(center_x, center_y, radius);

    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA_BOLD, 14);
    std::ostringstream state_label;
    state_label << "S" << state;
    const std::string state_text = state_label.str();
    fl_draw(state_text.c_str(), center_x - 12, center_y - 2);

    fl_font(FL_HELVETICA, 10);
    const std::string probability_text = format_probability(probability);
    fl_draw(("p=" + probability_text).c_str(), center_x - 22, center_y + radius + 17);
}

void StateGraphWidget::draw_arrow(int x1, int y1, int x2, int y2, bool upper) const {
    const int offset = upper ? -24 : 24;
    const int line_y = y1 + offset;
    fl_color(fl_rgb_color(85, 85, 85));
    fl_line_style(FL_SOLID, 1);
    fl_line(x1, line_y, x2, line_y);

    const int direction = x2 >= x1 ? 1 : -1;
    fl_line(x2, line_y, x2 - direction * 7, line_y - 4);
    fl_line(x2, line_y, x2 - direction * 7, line_y + 4);

    fl_font(FL_HELVETICA, 10);
    const char* rate = upper ? "(n-k)lambda" : "k*mu";
    const int text_width = static_cast<int>(std::strlen(rate)) * 5;
    fl_draw(rate, (x1 + x2 - text_width) / 2, line_y + (upper ? -5 : 14));
}

void StateGraphWidget::draw() {
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());
    fl_color(fl_rgb_color(160, 160, 160));
    fl_rect(x(), y(), w(), h());

    fl_color(fl_rgb_color(42, 77, 105));
    fl_font(FL_HELVETICA_BOLD, 14);
    fl_draw("Граф состояний", x() + 14, y() + 25);

    if (probabilities_.empty()) {
        fl_color(fl_rgb_color(100, 100, 100));
        fl_font(FL_HELVETICA, 13);
        fl_draw("Введите параметры и нажмите «Рассчитать»", x() + 28, y() + h() / 2);
        return;
    }

    std::vector<int> visible_states;
    const int state_count = static_cast<int>(probabilities_.size());
    if (state_count <= kMaxVisibleStates) {
        for (int state = 0; state < state_count; ++state) {
            visible_states.push_back(state);
        }
    } else {
        for (int state = 0; state < 8; ++state) {
            visible_states.push_back(state);
        }
        visible_states.push_back(-1);
        for (int state = state_count - 8; state < state_count; ++state) {
            visible_states.push_back(state);
        }
    }

    const int center_y = y() + h() / 2 + 10;
    const int usable_width = std::max(1, w() - 2 * kHorizontalMargin);
    const int gap = visible_states.size() > 1
        ? usable_width / static_cast<int>(visible_states.size() - 1)
        : usable_width;

    std::vector<int> centers;
    centers.reserve(visible_states.size());
    for (size_t index = 0; index < visible_states.size(); ++index) {
        centers.push_back(x() + kHorizontalMargin + static_cast<int>(index) * gap);
    }

    for (size_t index = 0; index + 1 < visible_states.size(); ++index) {
        if (visible_states[index] < 0 || visible_states[index + 1] < 0) {
            continue;
        }
        draw_arrow(centers[index] + kRadius, center_y, centers[index + 1] - kRadius, center_y, true);
        draw_arrow(centers[index + 1] - kRadius, center_y, centers[index] + kRadius, center_y, false);
    }

    for (size_t index = 0; index < visible_states.size(); ++index) {
        const int state = visible_states[index];
        if (state < 0) {
            fl_color(fl_rgb_color(75, 75, 75));
            fl_font(FL_HELVETICA_BOLD, 20);
            fl_draw("...", centers[index] - 13, center_y + 7);
            continue;
        }
        draw_state(state, centers[index], center_y, kRadius, probabilities_[state]);
    }

    fl_color(fl_rgb_color(90, 90, 90));
    fl_font(FL_HELVETICA, 11);
    fl_draw("При k неисправных станках: рождение (n-k)lambda, ремонт k*mu", x() + 14, y() + h() - 13);
}
