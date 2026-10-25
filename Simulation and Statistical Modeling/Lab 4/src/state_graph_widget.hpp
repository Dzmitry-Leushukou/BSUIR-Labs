#pragma once

#include <FL/Fl_Widget.H>

#include <vector>

class StateGraphWidget final : public Fl_Widget {
public:
    StateGraphWidget(int x, int y, int w, int h, const char* label = nullptr);

    void set_probabilities(const std::vector<double>& probabilities);
    void clear();
    void draw() override;

private:
    std::vector<double> probabilities_;

    void draw_state(int state, int center_x, int center_y, int radius, double probability) const;
    void draw_arrow(int x1, int y1, int x2, int y2, bool upper) const;
};
