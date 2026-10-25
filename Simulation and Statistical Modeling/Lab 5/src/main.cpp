#include "petri_ui.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>

#include <vector>

namespace {

std::vector<PlacePosition> leftPlaces() {
    return {
        {205, 125}, {505, 125}, {505, 405}, {205, 405}, {75, 585},
    };
}

std::vector<TransitionPosition> leftTransitions() {
    return {
        {110, 270, true}, {600, 270, true}, {355, 270, true}, {115, 485, false},
    };
}

std::vector<PetriArc> leftArcs() {
    return {
        {true, 3, false, 0},  {false, 0, true, 0},
        {true, 2, false, 1},  {false, 1, true, 1},
        {true, 0, false, 2},  {true, 1, false, 2},
        {false, 2, true, 3},  {false, 2, true, 2},
        {true, 3, false, 3},  {false, 3, true, 4},
    };
}

std::vector<PlacePosition> rightPlaces() {
    return {
        {75, 350}, {250, 160}, {250, 540}, {500, 160}, {500, 540}, {365, 350},
    };
}

std::vector<TransitionPosition> rightTransitions() {
    return {
        {160, 350, false}, {375, 160, false}, {375, 540, false}, {620, 350, false},
    };
}

std::vector<PetriArc> rightArcs() {
    return {
        {true, 0, false, 0},
        {false, 0, true, 1}, {false, 0, true, 5}, {false, 0, true, 2},
        {true, 1, false, 1}, {false, 1, true, 3},
        {true, 2, false, 2}, {false, 2, true, 4},
        {true, 3, false, 3}, {true, 4, false, 3}, {true, 5, false, 3},
        {false, 3, true, 0},
    };
}

} // namespace

int main() {
    Fl::scheme("gtk+");
    Fl::visual(FL_DOUBLE | FL_RGB);

    Fl_Window window(1340, 900, "Лабораторная работа №5 — Сети Петри");
    window.color(fl_rgb_color(241, 245, 249));
    window.resizable(window);

    Fl_Tabs tabs(8, 8, 1324, 884);
    tabs.selection_color(fl_rgb_color(203, 213, 225));
    tabs.begin();

    NetworkPage first(8, 33, 1324, 850, makeLeftNetwork(),
                      leftPlaces(), leftTransitions(), leftArcs(), "Схема 1");
    NetworkPage second(8, 33, 1324, 850, makeRightNetwork(),
                       rightPlaces(), rightTransitions(), rightArcs(), "Схема 2");

    tabs.end();
    window.end();
    window.show();
    return Fl::run();
}
