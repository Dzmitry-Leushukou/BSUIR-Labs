#include "petri_ui.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

constexpr double kPlaceRadius = 31.0;
constexpr double kTransitionHalfWidth = 9.0;
constexpr double kTransitionHalfHeight = 34.0;

std::string markingDetails(const PetriNet& net) {
    std::ostringstream out;
    out << "M = " << PetriNet::formatMarking(net.marking) << "\n";
    for (std::size_t i = 0; i < net.places.size(); ++i) {
        out << net.places[i] << " = " << net.marking[i];
        if (i + 1 < net.places.size()) {
            out << "    ";
        }
    }
    return out.str();
}

std::string enabledDetails(const PetriNet& net) {
    const auto enabled = net.enabledTransitions(net.marking);
    std::ostringstream out;
    out << "Доступные переходы: ";
    if (enabled.empty()) {
        out << "нет";
        return out.str();
    }
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        if (i != 0) out << ", ";
        out << net.transitions[enabled[i]].name;
    }
    return out.str();
}

} // namespace

PetriCanvas::PetriCanvas(int x, int y, int width, int height, PetriNet* net,
                         std::vector<PlacePosition> places,
                         std::vector<TransitionPosition> transitions,
                         std::vector<PetriArc> arcs)
    : Fl_Widget(x, y, width, height),
      net_(net),
      places_(std::move(places)),
      transitions_(std::move(transitions)),
      arcs_(std::move(arcs)) {
    box(FL_DOWN_BOX);
    color(FL_WHITE);
}

void PetriCanvas::redrawModel() {
    redraw();
}

PetriCanvas::Point PetriCanvas::endpoint(bool isPlace, int index) const {
    if (isPlace) {
        return {x() + places_[index].x, y() + places_[index].y};
    }
    return {x() + transitions_[index].x, y() + transitions_[index].y};
}

void PetriCanvas::drawArc(const PetriArc& arc) const {
    const Point from = endpoint(arc.fromPlace, arc.from);
    const Point to = endpoint(arc.toPlace, arc.to);
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    const double distance = std::max(1.0, std::sqrt(dx * dx + dy * dy));
    const double ux = dx / distance;
    const double uy = dy / distance;
    const double startOffset = arc.fromPlace ? kPlaceRadius : 11.0;
    const double endOffset = arc.toPlace ? kPlaceRadius : 11.0;
    const Point start{from.x + ux * startOffset, from.y + uy * startOffset};
    const Point end{to.x - ux * endOffset, to.y - uy * endOffset};

    fl_color(fl_rgb_color(71, 85, 105));
    fl_line_style(FL_SOLID, 2);
    fl_line(static_cast<int>(start.x), static_cast<int>(start.y),
            static_cast<int>(end.x), static_cast<int>(end.y));

    const double arrowLength = 10.0;
    const double arrowWidth = 5.0;
    const Point left{end.x - ux * arrowLength - uy * arrowWidth,
                     end.y - uy * arrowLength + ux * arrowWidth};
    const Point right{end.x - ux * arrowLength + uy * arrowWidth,
                      end.y - uy * arrowLength - ux * arrowWidth};
    fl_begin_polygon();
    fl_vertex(static_cast<int>(end.x), static_cast<int>(end.y));
    fl_vertex(static_cast<int>(left.x), static_cast<int>(left.y));
    fl_vertex(static_cast<int>(right.x), static_cast<int>(right.y));
    fl_end_polygon();
    fl_line_style(0);
}

void PetriCanvas::drawPlace(int index) const {
    const auto& place = places_[index];
    const int x = static_cast<int>(this->x() + place.x);
    const int y = static_cast<int>(this->y() + place.y);
    fl_color(fl_rgb_color(248, 250, 252));
    fl_pie(x - static_cast<int>(kPlaceRadius), y - static_cast<int>(kPlaceRadius),
           static_cast<int>(kPlaceRadius * 2), static_cast<int>(kPlaceRadius * 2), 0, 360);
    fl_color(fl_rgb_color(30, 41, 59));
    fl_line_style(FL_SOLID, 2);
    fl_arc(x - static_cast<int>(kPlaceRadius), y - static_cast<int>(kPlaceRadius),
           static_cast<int>(kPlaceRadius * 2), static_cast<int>(kPlaceRadius * 2), 0, 360);
    fl_line_style(0);

    const int tokens = net_->marking[index];
    fl_color(fl_rgb_color(22, 101, 52));
    if (tokens <= 3) {
        if (tokens == 1) {
            fl_pie(x - 7, y - 7, 14, 14, 0, 360);
        } else if (tokens == 2) {
            fl_pie(x - 14, y - 7, 12, 12, 0, 360);
            fl_pie(x + 2, y - 7, 12, 12, 0, 360);
        } else if (tokens == 3) {
            fl_pie(x - 6, y - 13, 12, 12, 0, 360);
            fl_pie(x - 14, y + 3, 12, 12, 0, 360);
            fl_pie(x + 2, y + 3, 12, 12, 0, 360);
        }
    } else {
        fl_font(FL_HELVETICA_BOLD, 18);
        const std::string value = std::to_string(tokens);
        fl_draw(value.c_str(), x - 6, y + 6);
    }

    fl_color(fl_rgb_color(15, 23, 42));
    fl_font(FL_HELVETICA_BOLD, 15);
    fl_draw(net_->places[index].c_str(), x - 10, y + static_cast<int>(kPlaceRadius) + 22);
}

void PetriCanvas::drawTransition(int index) const {
    const auto& transition = transitions_[index];
    const int x = static_cast<int>(this->x() + transition.x);
    const int y = static_cast<int>(this->y() + transition.y);
    const bool enabled = net_->isEnabled(index, net_->marking);
    const int width = transition.horizontal ? static_cast<int>(kTransitionHalfHeight * 2) :
                                              static_cast<int>(kTransitionHalfWidth * 2);
    const int height = transition.horizontal ? static_cast<int>(kTransitionHalfWidth * 2) :
                                               static_cast<int>(kTransitionHalfHeight * 2);

    fl_color(enabled ? fl_rgb_color(22, 163, 74) : fl_rgb_color(30, 41, 59));
    fl_rectf(x - width / 2, y - height / 2, width, height);
    fl_color(FL_WHITE);
    fl_rect(x - width / 2, y - height / 2, width, height);

    fl_color(fl_rgb_color(15, 23, 42));
    fl_font(FL_HELVETICA_BOLD, 15);
    fl_draw(net_->transitions[index].name.c_str(), x - 9, y + height / 2 + 22);
}

void PetriCanvas::draw() {
    fl_push_clip(x(), y(), w(), h());
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());

    fl_color(fl_rgb_color(71, 85, 105));
    fl_font(FL_HELVETICA_BOLD, 16);
    fl_draw("Схема сети Петри", x() + 18, y() + 30);

    for (const auto& arc : arcs_) {
        drawArc(arc);
    }
    for (int place = 0; place < static_cast<int>(places_.size()); ++place) {
        drawPlace(place);
    }
    for (int transition = 0; transition < static_cast<int>(transitions_.size()); ++transition) {
        drawTransition(transition);
    }

    fl_pop_clip();
}

int PetriCanvas::handle(int event) {
    if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
        const double localX = Fl::event_x() - x();
        const double localY = Fl::event_y() - y();
        for (int transition = 0; transition < static_cast<int>(transitions_.size()); ++transition) {
            const auto& node = transitions_[transition];
            const double halfWidth = node.horizontal ? kTransitionHalfHeight : kTransitionHalfWidth;
            const double halfHeight = node.horizontal ? kTransitionHalfWidth : kTransitionHalfHeight;
            if (std::abs(localX - node.x) <= halfWidth + 12 &&
                std::abs(localY - node.y) <= halfHeight + 12) {
                if (onTransition) {
                    onTransition(transition);
                }
                return 1;
            }
        }
    }
    return Fl_Widget::handle(event);
}

NetworkPage::NetworkPage(int x, int y, int width, int height, PetriNet net,
                         std::vector<PlacePosition> places,
                         std::vector<TransitionPosition> transitions,
                         std::vector<PetriArc> arcs,
                         const char* tabLabel)
    : Fl_Group(x, y, width, height, tabLabel), net_(std::move(net)) {
    const int canvasWidth = std::min(730, width - 480);
    const int rightX = x + canvasWidth + 24;
    const int rightWidth = width - canvasWidth - 36;
    const int contentY = y + 8;
    const int canvasY = y + 48;
    const int canvasHeight = height - 60;

    begin();
    auto* heading = new Fl_Box(x + 14, contentY, canvasWidth - 10, 30,
                               "Моделирование и исследование сети Петри");
    heading->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    heading->labelsize(20);
    heading->labelfont(FL_HELVETICA_BOLD);

    canvas_ = new PetriCanvas(x + 10, canvasY, canvasWidth, canvasHeight,
                              &net_, std::move(places), std::move(transitions), std::move(arcs));
    canvas_->onTransition = [this](int transition) { fireTransition(transition); };

    auto* markingLabel = new Fl_Box(rightX, contentY, rightWidth, 22, "Текущая маркировка");
    markingLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    markingLabel->labelfont(FL_HELVETICA_BOLD);
    markingOutput_ = new Fl_Multiline_Output(rightX, contentY + 24, rightWidth, 58);
    markingOutput_->textfont(FL_COURIER);
    markingOutput_->textsize(14);

    auto* transitionLabel = new Fl_Box(rightX, contentY + 92, rightWidth, 22, "Срабатывание перехода");
    transitionLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    transitionLabel->labelfont(FL_HELVETICA_BOLD);
    transitionButtonData_.reserve(net_.transitions.size());
    transitionButtons_.reserve(net_.transitions.size());
    const int buttonWidth = std::max(70, (rightWidth - 12) / 2);
    for (int transition = 0; transition < static_cast<int>(net_.transitions.size()); ++transition) {
        const int row = transition / 2;
        const int column = transition % 2;
        auto* button = new Fl_Button(rightX + column * (buttonWidth + 8),
                                     contentY + 120 + row * 38, buttonWidth, 32,
                                     net_.transitions[transition].name.c_str());
        transitionButtonData_.push_back({this, transition});
        button->callback(transitionCallback, &transitionButtonData_.back());
        transitionButtons_.push_back(button);
    }

    const int actionY = contentY + 120 + ((static_cast<int>(net_.transitions.size()) + 1) / 2) * 38 + 8;
    auto* resetButton = new Fl_Button(rightX, actionY, buttonWidth, 34, "Сброс");
    resetButton->callback(resetCallback, this);
    autoButton_ = new Fl_Button(rightX + buttonWidth + 8, actionY, buttonWidth, 34, "Автошаг");
    autoButton_->callback(autoCallback, this);
    auto* nextButton = new Fl_Button(rightX, actionY + 42, buttonWidth, 34, "Следующий");
    nextButton->callback(nextCallback, this);
    auto* parallelButton = new Fl_Button(rightX + buttonWidth + 8, actionY + 42, buttonWidth, 34, "Параллельность");
    parallelButton->callback(parallelCallback, this);

    auto* targetLabel = new Fl_Box(rightX, actionY + 86, rightWidth, 22,
                                   "Целевая маркировка (через запятую)");
    targetLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    targetInput_ = new Fl_Input(rightX, actionY + 110, rightWidth - 110, 32);
    targetInput_->value(net_.places.size() == 5 ? "0,0,1,0,1" : "0,0,1,1,0,2");
    auto* checkButton = new Fl_Button(rightX + rightWidth - 102, actionY + 110, 102, 32, "Проверить");
    checkButton->callback(reachabilityCallback, this);

    const int resultsY = actionY + 152;
    auto* resultsTabs = new Fl_Tabs(rightX, resultsY, rightWidth, height - (resultsY - y) - 8);
    resultsTabs->selection_color(fl_rgb_color(203, 213, 225));
    resultsTabs->begin();
    auto* analysisGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Анализ");
    analysisGroup->begin();
    analysisOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                               height - (resultsY - y) - 40);
    analysisOutput_->textfont(FL_COURIER);
    analysisOutput_->textsize(12);
    analysisGroup->end();
    auto* reachGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Достижимость");
    reachGroup->begin();
    reachabilityOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                                   height - (resultsY - y) - 40);
    reachabilityOutput_->textfont(FL_COURIER);
    reachabilityOutput_->textsize(12);
    reachGroup->end();
    auto* parallelGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Параллельность");
    parallelGroup->begin();
    parallelOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                               height - (resultsY - y) - 40);
    parallelOutput_->textfont(FL_COURIER);
    parallelOutput_->textsize(12);
    parallelGroup->end();
    auto* graphGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Граф");
    graphGroup->begin();
    graphOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                            height - (resultsY - y) - 40);
    graphOutput_->textfont(FL_COURIER);
    graphOutput_->textsize(12);
    graphGroup->end();
    auto* logGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Журнал");
    logGroup->begin();
    logOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                          height - (resultsY - y) - 40);
    logOutput_->textfont(FL_COURIER);
    logOutput_->textsize(12);
    logGroup->end();
    resultsTabs->end();
    end();

    updateView();
    showParallel();
    showGraph();
}

NetworkPage::~NetworkPage() {
    if (autoRunning_) {
        Fl::remove_timeout(autoTimer, this);
    }
}

void NetworkPage::transitionCallback(Fl_Widget*, void* data) {
    const auto* callbackData = static_cast<TransitionButtonData*>(data);
    callbackData->page->fireTransition(callbackData->transition);
}

void NetworkPage::resetCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->reset();
}

void NetworkPage::autoCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->toggleAuto();
}

void NetworkPage::nextCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->nextAvailable();
}

void NetworkPage::reachabilityCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->checkReachability();
}

void NetworkPage::parallelCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->showParallel();
}

void NetworkPage::graphCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->showGraph();
}

void NetworkPage::autoTimer(void* data) {
    auto* page = static_cast<NetworkPage*>(data);
    if (!page->autoRunning_) {
        return;
    }
    const auto enabled = page->net_.enabledTransitions(page->net_.marking);
    if (enabled.empty()) {
        page->toggleAuto();
        return;
    }
    page->fireTransition(enabled.front());
    if (page->autoRunning_) {
        Fl::repeat_timeout(0.65, autoTimer, page);
    }
}

void NetworkPage::fireTransition(int transition) {
    if (net_.fire(transition, net_.marking)) {
        appendLog("Сработал переход " + net_.transitions[transition].name +
                  ", новая маркировка " + PetriNet::formatMarking(net_.marking));
        updateView();
        showParallel();
    } else {
        appendLog("Переход " + net_.transitions[transition].name + " сейчас недоступен.");
    }
}

void NetworkPage::reset() {
    if (autoRunning_) {
        toggleAuto();
    }
    net_.marking = net_.initial;
    appendLog("Состояние сброшено к начальной маркировке.");
    updateView();
    showParallel();
}

void NetworkPage::toggleAuto() {
    autoRunning_ = !autoRunning_;
    autoButton_->label(autoRunning_ ? "Стоп" : "Автошаг");
    if (autoRunning_) {
        Fl::add_timeout(0.05, autoTimer, this);
    } else {
        Fl::remove_timeout(autoTimer, this);
    }
    autoButton_->redraw();
}

void NetworkPage::nextAvailable() {
    const auto enabled = net_.enabledTransitions(net_.marking);
    if (enabled.empty()) {
        appendLog("Нет доступных переходов: достигнута тупиковая маркировка.");
        return;
    }
    fireTransition(enabled.front());
}

void NetworkPage::checkReachability() {
    PetriNet::Marking target;
    std::string error;
    if (!PetriNet::parseMarking(targetInput_->value(), net_.places.size(), target, error)) {
        reachabilityOutput_->value(("Ошибка ввода: " + error).c_str());
        return;
    }

    std::vector<int> path;
    std::ostringstream out;
    out << "Начальная: " << PetriNet::formatMarking(net_.initial) << "\n"
        << "Целевая:   " << PetriNet::formatMarking(target) << "\n\n";
    if (net_.findPath(target, path)) {
        out << "Маркировка достижима.\nПуть: ";
        if (path.empty()) {
            out << "пустой (это начальная маркировка)";
        } else {
            for (std::size_t i = 0; i < path.size(); ++i) {
                if (i != 0) out << " -> ";
                out << net_.transitions[path[i]].name;
            }
        }
    } else {
        out << "Маркировка не обнаружена в графе достижимости.";
    }
    reachabilityOutput_->value(out.str().c_str());
}

void NetworkPage::showParallel() {
    const auto sets = net_.parallelSets(net_.marking);
    std::ostringstream out;
    out << "Текущая маркировка: " << PetriNet::formatMarking(net_.marking) << "\n\n";
    if (sets.empty()) {
        out << "Двух и более переходов, которые можно\n"
               "сработать одновременно, сейчас нет.";
    } else {
        out << "Допустимые параллельные наборы:\n";
        for (const auto& set : sets) {
            out << "{ ";
            for (std::size_t i = 0; i < set.size(); ++i) {
                if (i != 0) out << ", ";
                out << net_.transitions[set[i]].name;
            }
            out << " }\n";
        }
    }
    parallelOutput_->value(out.str().c_str());
}

void NetworkPage::showGraph() {
    graphOutput_->value(net_.graphDescription().c_str());
}

void NetworkPage::updateView() {
    markingOutput_->value((markingDetails(net_) + "\n" + enabledDetails(net_)).c_str());
    analysisOutput_->value(net_.analysis().c_str());
    canvas_->redrawModel();
    for (int transition = 0; transition < static_cast<int>(transitionButtons_.size()); ++transition) {
        const bool enabled = net_.isEnabled(transition, net_.marking);
        transitionButtons_[transition]->color(enabled ? fl_rgb_color(187, 247, 208) : FL_BACKGROUND_COLOR);
        transitionButtons_[transition]->labelcolor(enabled ? fl_rgb_color(20, 83, 45) : FL_FOREGROUND_COLOR);
        transitionButtons_[transition]->redraw();
    }
}

void NetworkPage::appendLog(const std::string& line) {
    if (!log_.empty()) {
        log_ += "\n";
    }
    log_ += line;
    logOutput_->value(log_.c_str());
}
