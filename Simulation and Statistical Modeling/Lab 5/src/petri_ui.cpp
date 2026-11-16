#include "petri_ui.hpp"
#include "petri_experiment.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace {

constexpr double kPlaceRadius = 31.0;
constexpr double kTransitionHalfWidth = 9.0;
constexpr double kTransitionHalfHeight = 34.0;

template <typename Integer>
bool parseInteger(const char* text, Integer& value) {
    if (text == nullptr || *text == '\0') return false;
    const std::string_view input(text);
    const auto result = std::from_chars(input.data(), input.data() + input.size(), value);
    return result.ec == std::errc{} && result.ptr == input.data() + input.size();
}

void writeComparison(std::ostringstream& out, double theoretical, double experimental,
                     bool percentage = false) {
    const double scale = percentage ? 100.0 : 1.0;
    out << std::right << std::fixed << std::setprecision(percentage ? 2 : 3)
        << std::setw(9) << theoretical * scale;
    if (percentage) out << '%';
    out << std::setw(12) << experimental * scale;
    if (percentage) out << '%';
}

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

ReachabilityCanvas::ReachabilityCanvas(int x, int y, int width, int height,
                                       const PetriNet* net)
    : Fl_Widget(x, y, width, height), net_(net), graph_(net->buildReachability()) {
    box(FL_DOWN_BOX);
    color(FL_WHITE);
    layout();
}

void ReachabilityCanvas::redrawGraph() {
    graph_ = net_->buildReachability();
    layout();
    redraw();
}

void ReachabilityCanvas::layout() {
    positions_.clear();
    if (graph_.markings.empty()) return;

    if (net_->places.size() == 6 && graph_.markings.size() == 5) {
        // Put the two independent branches above and below the main path.
        // Leave room under the nodes for t4 returning to the initial state.
        nodeRadius_ = std::clamp(std::min(w() / 12.0, (h() - 70.0) / 4.0), 20.0, 45.0);
        const double left = x() + nodeRadius_ + 14;
        const double top = y() + 36 + nodeRadius_;
        const double spanX = std::max(1.0, w() - 2 * nodeRadius_ - 28);
        const double spanY = std::max(1.0, h() - 2 * nodeRadius_ - 76);
        for (const auto& slot : std::vector<Point>{{0, 0.5}, {0.27, 0.5},
                                                  {0.63, 0}, {0.63, 1}, {1, 0.5}}) {
            positions_.push_back({left + slot.x * spanX, top + slot.y * spanY});
        }
        return;
    }
    nodeRadius_ = 45.0;

    const int columns = std::max(1, static_cast<int>(std::ceil(std::sqrt(
        static_cast<double>(graph_.markings.size())))));
    const int rows = (static_cast<int>(graph_.markings.size()) + columns - 1) / columns;
    const double cellWidth = static_cast<double>(std::max(1, w() - 44)) / columns;
    const double cellHeight = static_cast<double>(std::max(1, h() - 52)) / rows;
    positions_.reserve(graph_.markings.size());
    for (std::size_t i = 0; i < graph_.markings.size(); ++i) {
        const int row = static_cast<int>(i) / columns;
        const int column = static_cast<int>(i) % columns;
        positions_.push_back({x() + 22.0 + (column + 0.5) * cellWidth,
                              y() + 30.0 + (row + 0.5) * cellHeight});
    }
}

std::string ReachabilityCanvas::edgeLabel(int from, int to) const {
    for (int transition = 0; transition < static_cast<int>(net_->transitions.size()); ++transition) {
        auto candidate = graph_.markings[from];
        if (net_->fire(transition, candidate) &&
            candidate == graph_.markings[to]) {
            return net_->transitions[transition].name;
        }
    }
    return "";
}

void ReachabilityCanvas::drawArrow(Point from, Point to, const std::string& label,
                                   bool returnBelow) const {
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    const double distance = std::max(1.0, std::sqrt(dx * dx + dy * dy));
    double ux = dx / distance;
    double uy = dy / distance;
    const double radius = nodeRadius_;
    Point labelPoint{(from.x + to.x) / 2.0, (from.y + to.y) / 2.0 - 9};
    if (returnBelow) {
        from.y += radius;
        to.y += radius;
        ux = 0;
        uy = -1;
        labelPoint.y = y() + h() - 16;
    } else {
    from.x += ux * radius;
    from.y += uy * radius;
    to.x -= ux * radius;
    to.y -= uy * radius;
    }
    fl_color(fl_rgb_color(100, 116, 139));
    fl_line_style(FL_SOLID, 2);
    if (returnBelow) {
        const int bottom = y() + h() - 16;
        fl_line(static_cast<int>(from.x), static_cast<int>(from.y),
                static_cast<int>(from.x), bottom);
        fl_line(static_cast<int>(from.x), bottom, static_cast<int>(to.x), bottom);
        fl_line(static_cast<int>(to.x), bottom, static_cast<int>(to.x), static_cast<int>(to.y));
    } else {
        fl_line(static_cast<int>(from.x), static_cast<int>(from.y),
                static_cast<int>(to.x), static_cast<int>(to.y));
    }
    const Point left{to.x - ux * 9.0 - uy * 4.5, to.y - uy * 9.0 + ux * 4.5};
    const Point right{to.x - ux * 9.0 + uy * 4.5, to.y - uy * 9.0 - ux * 4.5};
    fl_begin_polygon();
    fl_vertex(static_cast<int>(to.x), static_cast<int>(to.y));
    fl_vertex(static_cast<int>(left.x), static_cast<int>(left.y));
    fl_vertex(static_cast<int>(right.x), static_cast<int>(right.y));
    fl_end_polygon();
    fl_line_style(0);
    if (!label.empty()) {
        fl_font(FL_HELVETICA, 12);
        const int labelWidth = static_cast<int>(fl_width(label.c_str())) + 8;
        fl_color(FL_WHITE);
        fl_rectf(static_cast<int>(labelPoint.x) - labelWidth / 2,
                 static_cast<int>(labelPoint.y) - 11, labelWidth, 16);
        fl_color(fl_rgb_color(30, 41, 59));
        fl_draw(label.c_str(), static_cast<int>(labelPoint.x) - labelWidth / 2 + 4,
                static_cast<int>(labelPoint.y) + 1);
    }
}

void ReachabilityCanvas::draw() {
    // FLTK can move/resize a hidden tab before it is drawn again.
    // Coordinates must always come from the current widget geometry.
    layout();
    fl_push_clip(x(), y(), w(), h());
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());
    fl_color(fl_rgb_color(71, 85, 105));
    fl_font(FL_HELVETICA_BOLD, 14);
    fl_draw("Диаграмма маркировок (граф достижимости)", x() + 12, y() + 20);

    for (const auto& edge : graph_.edges) {
        if (edge.first < static_cast<int>(positions_.size()) &&
            edge.second < static_cast<int>(positions_.size()) && edge.first != edge.second) {
            drawArrow(positions_[edge.first], positions_[edge.second],
                      edgeLabel(edge.first, edge.second),
                      net_->places.size() == 6 && graph_.markings.size() == 5 &&
                      edge.first == 4 && edge.second == 0);
        }
    }
    for (std::size_t i = 0; i < positions_.size(); ++i) {
        const auto point = positions_[i];
        fl_color(i == 0 ? fl_rgb_color(220, 252, 231) : fl_rgb_color(239, 246, 255));
        const int diameter = static_cast<int>(nodeRadius_ * 2);
        fl_pie(static_cast<int>(point.x - nodeRadius_), static_cast<int>(point.y - nodeRadius_),
               diameter, diameter, 0, 360);
        fl_color(fl_rgb_color(30, 41, 59));
        fl_line_style(FL_SOLID, 2);
        fl_arc(static_cast<int>(point.x - nodeRadius_), static_cast<int>(point.y - nodeRadius_),
               diameter, diameter, 0, 360);
        fl_line_style(0);
        fl_font(FL_HELVETICA_BOLD, 13);
        const std::string name = "M" + std::to_string(i);
        fl_draw(name.c_str(), static_cast<int>(point.x - 10), static_cast<int>(point.y - 4));
        fl_font(FL_COURIER, 9);
        std::string marking = PetriNet::formatMarking(graph_.markings[i]);
        marking.erase(std::remove(marking.begin(), marking.end(), ' '), marking.end());
        fl_draw(marking.c_str(), static_cast<int>(point.x - fl_width(marking.c_str()) / 2),
                static_cast<int>(point.y + 15));
    }
    if (!graph_.complete) {
        fl_color(fl_rgb_color(153, 27, 27));
        fl_font(FL_HELVETICA, 12);
        fl_draw("Граф ограничен лимитом состояний", x() + 12, y() + h() - 10);
    }
    fl_pop_clip();
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
    const int resultHeight = height - (resultsY - y) - 32;
    const int graphViewportY = resultsY + 28;
    const int graphViewportHeight = std::max(120, resultHeight - 4);
    const int graphCanvasHeight = std::max(360, resultHeight - 90);
    const int graphTextHeight = std::max(260, resultHeight - 70);
    graphScroll_ = new Fl_Scroll(rightX + 2, graphViewportY, rightWidth - 4, graphViewportHeight);
    graphScroll_->type(Fl_Scroll::VERTICAL_ALWAYS);
    graphScroll_->scroll_to(0, 0);
    graphScroll_->begin();
    graphCanvas_ = new ReachabilityCanvas(rightX + 6, graphViewportY + 4, rightWidth - 12,
                                          graphCanvasHeight, &net_);
    graphBuffer_ = new Fl_Text_Buffer();
    graphOutput_ = new Fl_Text_Display(rightX + 6, graphViewportY + graphCanvasHeight + 8,
                                        rightWidth - 12, graphTextHeight);
    graphOutput_->buffer(graphBuffer_);
    graphOutput_->textfont(FL_COURIER);
    graphOutput_->textsize(10);
    graphScroll_->end();
    graphGroup->end();
    auto* logGroup = new Fl_Group(rightX, resultsY + 24, rightWidth, height - (resultsY - y) - 32, "Журнал");
    logGroup->begin();
    logOutput_ = new Fl_Multiline_Output(rightX + 4, resultsY + 28, rightWidth - 8,
                                          height - (resultsY - y) - 40);
    logOutput_->textfont(FL_COURIER);
    logOutput_->textsize(12);
    logGroup->end();
    auto* experimentGroup = new Fl_Group(rightX, resultsY + 24, rightWidth,
                                         height - (resultsY - y) - 32, "Симуляция");
    experimentGroup->begin();
    int controlX = rightX + 8;
    const int controlY = resultsY + 32;
    auto* runsLabel = new Fl_Box(controlX, controlY, 62, 28, "Прогоны");
    runsLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    runsInput_ = new Fl_Input(controlX + 62, controlY, 68, 30);
    runsInput_->value("10000");
    controlX += 138;
    auto* stepsLabel = new Fl_Box(controlX, controlY, 42, 28, "Шаги");
    stepsLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    stepsInput_ = new Fl_Input(controlX + 42, controlY, 62, 30);
    stepsInput_->value("30");
    controlX += 112;
    auto* seedLabel = new Fl_Box(controlX, controlY, 34, 28, "Seed");
    seedLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    seedInput_ = new Fl_Input(controlX + 34, controlY, 76, 30);
    seedInput_->value("17");
    controlX += 118;
    auto* experimentButton = new Fl_Button(controlX, controlY, 104, 30, "Запустить");
    experimentButton->callback(experimentCallback, this);
    experimentBuffer_ = new Fl_Text_Buffer();
    experimentOutput_ = new Fl_Text_Display(rightX + 4, resultsY + 68, rightWidth - 8,
                                             height - (resultsY - y) - 76);
    experimentOutput_->buffer(experimentBuffer_);
    experimentOutput_->textfont(FL_COURIER);
    experimentOutput_->textsize(11);
    experimentOutput_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    experimentGroup->end();
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
    if (experimentOutput_ != nullptr) {
        experimentOutput_->buffer(nullptr);
    }
    delete experimentBuffer_;
    if (graphOutput_ != nullptr) {
        graphOutput_->buffer(nullptr);
    }
    delete graphBuffer_;
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

void NetworkPage::experimentCallback(Fl_Widget*, void* data) {
    static_cast<NetworkPage*>(data)->runExperiment();
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
    graphBuffer_->text(net_.graphDescription().c_str());
    graphOutput_->scroll(1, 0);
    graphCanvas_->redrawGraph();
}

void NetworkPage::runExperiment() {
    ExperimentOptions options;
    std::uint32_t seed = 0;
    if (!parseInteger(runsInput_->value(), options.runs) ||
        !parseInteger(stepsInput_->value(), options.maxSteps) ||
        !parseInteger(seedInput_->value(), seed)) {
        experimentBuffer_->text("Введите целые числа для прогонов, шагов и seed.");
        return;
    }
    options.seed = seed;
    std::string error;
    if (!PetriNet::parseMarking(targetInput_->value(), net_.places.size(), options.target, error)) {
        experimentBuffer_->text(("Ошибка целевой маркировки: " + error).c_str());
        return;
    }

    try {
        PetriExperiment experiment(net_, options);
        experiment.runBatch(options.runs);
        const auto& exact = experiment.exact();
        const auto& samples = experiment.samples();
        const auto& graph = experiment.graph();
        std::ostringstream out;
        out << "Переход равновероятно выбирается среди доступных.\n"
            << "Горизонт: " << options.maxSteps << " шагов; тупик завершает прогон.\n"
            << "Цель: " << PetriNet::formatMarking(options.target) << "\n\n"
            << "Конечная маркировка              Теория      Опыт\n";
        for (std::size_t state = 0; state < graph.markings.size(); ++state) {
            out << std::left << std::setw(31) << PetriNet::formatMarking(graph.markings[state]);
            writeComparison(out, exact.finalProbabilities[state],
                            static_cast<double>(samples.finalCounts[state]) / samples.completedRuns, true);
            out << '\n';
        }
        out << "\nПоказатель                         Теория      Опыт\n";
        out << std::left << std::setw(31) << "Тупиковый конечный исход";
        writeComparison(out, exact.deadlockProbability,
                        static_cast<double>(samples.deadlockRuns) / samples.completedRuns, true);
        out << '\n' << std::left << std::setw(31) << "Цель посещалась за прогон";
        writeComparison(out, exact.targetHitProbability,
                        static_cast<double>(samples.targetHitRuns) / samples.completedRuns, true);
        out << '\n' << std::left << std::setw(31) << "Параллельность в конце";
        writeComparison(out, exact.parallelFinalProbability,
                        static_cast<double>(samples.parallelFinalRuns) / samples.completedRuns, true);
        out << '\n' << std::left << std::setw(31) << "Срабатываний за прогон";
        writeComparison(out, exact.meanFirings,
                        static_cast<double>(samples.firings) / samples.completedRuns);
        out << "\n\nСрабатываний переходов за прогон\n";
        for (std::size_t transition = 0; transition < net_.transitions.size(); ++transition) {
            out << std::left << std::setw(8) << net_.transitions[transition].name;
            writeComparison(out, exact.transitionMeans[transition],
                            static_cast<double>(samples.transitionCounts[transition]) / samples.completedRuns);
            out << '\n';
        }
        out << "\nСреднее число фишек в конце (по позициям)\n";
        for (std::size_t place = 0; place < net_.places.size(); ++place) {
            double experimentalMean = 0;
            for (std::size_t state = 0; state < graph.markings.size(); ++state) {
                experimentalMean += static_cast<double>(samples.finalCounts[state]) * graph.markings[state][place];
            }
            experimentalMean /= samples.completedRuns;
            out << std::left << std::setw(8) << net_.places[place];
            writeComparison(out, exact.finalTokenMeans[place], experimentalMean);
            out << '\n';
        }
        out << "\nМаксимум фишек за время наблюдения\n";
        for (std::size_t place = 0; place < net_.places.size(); ++place) {
            out << std::left << std::setw(8) << net_.places[place]
                << "граф " << exact.reachableMaxTokens[place]
                << ", эксперимент " << samples.observedMaxTokens[place] << '\n';
        }
        out << "\nСерия: " << samples.completedRuns << " прогонов, seed=" << options.seed;
        experimentBuffer_->text(out.str().c_str());
        experimentOutput_->scroll(0, 0);
    } catch (const std::exception& exception) {
        experimentBuffer_->text((std::string("Не удалось выполнить симуляцию: ") + exception.what()).c_str());
    }
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
