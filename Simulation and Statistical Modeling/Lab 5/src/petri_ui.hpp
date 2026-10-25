#pragma once

#include "petri_model.hpp"

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include <functional>
#include <string>
#include <vector>

struct PlacePosition {
    double x = 0.0;
    double y = 0.0;
};

struct TransitionPosition {
    double x = 0.0;
    double y = 0.0;
    bool horizontal = false;
};

struct PetriArc {
    bool fromPlace = true;
    int from = 0;
    bool toPlace = false;
    int to = 0;
};

class PetriCanvas final : public Fl_Widget {
public:
    PetriCanvas(int x, int y, int width, int height, PetriNet* net,
                std::vector<PlacePosition> places,
                std::vector<TransitionPosition> transitions,
                std::vector<PetriArc> arcs);

    std::function<void(int)> onTransition;
    void redrawModel();

protected:
    void draw() override;
    int handle(int event) override;

private:
    struct Point {
        double x;
        double y;
    };

    PetriNet* net_;
    std::vector<PlacePosition> places_;
    std::vector<TransitionPosition> transitions_;
    std::vector<PetriArc> arcs_;

    Point endpoint(bool isPlace, int index) const;
    void drawArc(const PetriArc& arc) const;
    void drawPlace(int index) const;
    void drawTransition(int index) const;
};

class NetworkPage final : public Fl_Group {
public:
    NetworkPage(int x, int y, int width, int height, PetriNet net,
                std::vector<PlacePosition> places,
                std::vector<TransitionPosition> transitions,
                std::vector<PetriArc> arcs,
                const char* tabLabel);
    ~NetworkPage() override;

private:
    struct TransitionButtonData {
        NetworkPage* page;
        int transition;
    };

    PetriNet net_;
    PetriCanvas* canvas_ = nullptr;
    Fl_Multiline_Output* markingOutput_ = nullptr;
    Fl_Multiline_Output* analysisOutput_ = nullptr;
    Fl_Multiline_Output* reachabilityOutput_ = nullptr;
    Fl_Multiline_Output* parallelOutput_ = nullptr;
    Fl_Multiline_Output* graphOutput_ = nullptr;
    Fl_Multiline_Output* logOutput_ = nullptr;
    Fl_Input* targetInput_ = nullptr;
    Fl_Button* autoButton_ = nullptr;
    std::vector<Fl_Button*> transitionButtons_;
    std::vector<TransitionButtonData> transitionButtonData_;
    std::string log_;
    bool autoRunning_ = false;

    static void transitionCallback(Fl_Widget* widget, void* data);
    static void resetCallback(Fl_Widget* widget, void* data);
    static void autoCallback(Fl_Widget* widget, void* data);
    static void nextCallback(Fl_Widget* widget, void* data);
    static void reachabilityCallback(Fl_Widget* widget, void* data);
    static void parallelCallback(Fl_Widget* widget, void* data);
    static void graphCallback(Fl_Widget* widget, void* data);
    static void autoTimer(void* data);

    void fireTransition(int transition);
    void reset();
    void toggleAuto();
    void nextAvailable();
    void checkReachability();
    void showParallel();
    void showGraph();
    void updateView();
    void appendLog(const std::string& line);
};
