#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include "task1.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class WheelOfFortune {
public:
    struct Game {
        std::string name;
        double amount;
        Fl_Color color;

        Game(const std::string& n, double a) : name(n), amount(a) {
            color = fl_rgb_color(rand() % 200 + 55, rand() % 200 + 55, rand() % 200 + 55);
        }
    };

    std::vector<Game> games;
    double total_amount;

    WheelOfFortune() : total_amount(0) {
        games.push_back(Game("Witcher 3", 250));
        games.push_back(Game("Cyberpunk 2077", 150));
        games.push_back(Game("Red Dead 2", 300));
        games.push_back(Game("GTA V", 100));
        recalculate_total();
    }

    void add_game(const std::string& name, double amount) {
        games.push_back(Game(name, amount));
        recalculate_total();
    }

    void remove_game(int index) {
        if (index >= 0 && index < (int)games.size()) {
            games.erase(games.begin() + index);
            recalculate_total();
        }
    }

    void update_game(int index, const std::string& name, double amount) {
        if (index >= 0 && index < (int)games.size()) {
            games[index].name = name;
            games[index].amount = amount;
            recalculate_total();
        }
    }

    void recalculate_total() {
        total_amount = 0;
        for (const auto& g : games) {
            total_amount += g.amount;
        }
    }

    int spin() {
        if (games.empty() || total_amount <= 0) return -1;

        double r = task1::runSingle(1.0) * total_amount;
        double cumsum = 0;

        for (int i = 0; i < (int)games.size(); i++) {
            cumsum += games[i].amount;
            if (r <= cumsum) {
                return i;
            }
        }

        return games.size() - 1;
    }
};

class WheelWidget : public Fl_Widget {
    WheelOfFortune* wheel_data;
    double rotation_angle;
    double target_angle;
    double spin_speed;
    bool is_spinning;
    int target_game_index;
    double arrow_angle;

public:
    WheelWidget(int X, int Y, int W, int H, WheelOfFortune* wd)
        : Fl_Widget(X, Y, W, H), wheel_data(wd), rotation_angle(0),
          target_angle(0), spin_speed(0), is_spinning(false),
          target_game_index(-1), arrow_angle(90) {}

    void start_spin(int target_index) {
        if (is_spinning || target_index < 0) return;

        target_game_index = target_index;
        is_spinning = true;
        spin_speed = 30.0;

        double sector_start = 0;
        for (int i = 0; i < target_index; i++) {
            sector_start += 360.0 * (wheel_data->games[i].amount / wheel_data->total_amount);
        }

        double sector_size = 360.0 * (wheel_data->games[target_index].amount / wheel_data->total_amount);
        double sector_center = sector_start + sector_size / 2.0;

        target_angle = rotation_angle + 720 + (arrow_angle - sector_center);

        while (target_angle - rotation_angle > 1080) target_angle -= 360;

        Fl::add_timeout(0.016, animate_callback, this);
    }

    static void animate_callback(void* data) {
        WheelWidget* w = (WheelWidget*)data;
        w->animate_step();
    }

    void animate_step() {
        if (!is_spinning) return;

        double remaining = target_angle - rotation_angle;

        if (std::abs(remaining) < 2.0) {
            rotation_angle = target_angle;
            is_spinning = false;
            redraw();

            while (rotation_angle >= 360) rotation_angle -= 360;
            while (rotation_angle < 0) rotation_angle += 360;

            return;
        }

        double move = remaining * 0.15;
        if (std::abs(move) < 0.5) {
            move = (move > 0) ? 0.5 : -0.5;
        }
        rotation_angle += move;

        redraw();
        Fl::add_timeout(0.016, animate_callback, this);
    }

    void draw() override {
        fl_color(fl_rgb_color(167, 191, 167));
        fl_rectf(x(), y(), w(), h());

        if (wheel_data->games.empty()) {
            fl_color(FL_BLACK);
            fl_font(FL_HELVETICA, 14);
            fl_draw("No games", x() + w()/2 - 30, y() + h()/2);
            return;
        }

        int cx = x() + w() / 2;
        int cy = y() + h() / 2;
        int radius = (std::min)(w(), h()) / 2 - 40;

        double start_angle = rotation_angle;

        for (const auto& game : wheel_data->games) {
            double sector_angle = 360.0 * (game.amount / wheel_data->total_amount);

            fl_color(game.color);
            fl_pie(cx - radius, cy - radius, radius * 2, radius * 2,
                   start_angle, start_angle + sector_angle);

            fl_color(FL_BLACK);
            fl_arc(cx - radius, cy - radius, radius * 2, radius * 2,
                   start_angle, start_angle + sector_angle);

            double text_angle = (start_angle + sector_angle / 2.0) * M_PI / 180.0;
            int text_x = cx + (int)(radius * 0.65 * cos(text_angle));
            int text_y = cy - (int)(radius * 0.65 * sin(text_angle));

            fl_font(FL_HELVETICA_BOLD, 14);

            fl_color(FL_WHITE);
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx != 0 || dy != 0) {
                        fl_draw(game.name.c_str(), text_x + dx - 40, text_y + dy);
                    }
                }
            }
            fl_color(FL_BLACK);
            fl_draw(game.name.c_str(), text_x - 40, text_y);

            start_angle += sector_angle;
        }

        fl_color(FL_BLACK);
        fl_line_style(FL_SOLID, 3);
        fl_arc(cx - radius, cy - radius, radius * 2, radius * 2, 0, 360);
        fl_line_style(0);

        fl_color(FL_WHITE);
        fl_pie(cx - 30, cy - 30, 60, 60, 0, 360);
        fl_color(FL_BLACK);
        fl_line_style(FL_SOLID, 2);
        fl_arc(cx - 30, cy - 30, 60, 60, 0, 360);
        fl_line_style(0);

        fl_color(FL_RED);
        fl_begin_polygon();
        fl_vertex(cx, cy - radius + 5);
        fl_vertex(cx - 15, cy - radius - 20);
        fl_vertex(cx + 15, cy - radius - 20);
        fl_end_polygon();
    }
};

class GamesTable : public Fl_Table {
    WheelOfFortune* wheel_data;

public:
    WheelWidget* wheel_widget;

    GamesTable(int X, int Y, int W, int H, WheelOfFortune* wd, WheelWidget* ww)
        : Fl_Table(X, Y, W, H), wheel_data(wd), wheel_widget(ww) {
        rows(wheel_data->games.size());
        row_header(0);
        row_height_all(25);
        row_resize(0);

        cols(4);
        col_header(1);
        col_width(0, 150);
        col_width(1, 80);
        col_width(2, 80);
        col_width(3, 70);
        col_resize(1);

        type(0);

        end();
    }

    void update_rows() {
        rows(wheel_data->games.size());
        redraw();
    }

protected:
    void draw_cell(TableContext context, int ROW, int COL,
                   int X, int Y, int W, int H) override {
        static char buffer[128];

        switch (context) {
            case CONTEXT_COL_HEADER:
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
                fl_color(FL_BLACK);
                fl_font(FL_HELVETICA, 14);
                if (COL == 0) fl_draw("Game", X, Y, W, H, FL_ALIGN_CENTER);
                else if (COL == 1) fl_draw("Amount", X, Y, W, H, FL_ALIGN_CENTER);
                else if (COL == 2) fl_draw("Prob %", X, Y, W, H, FL_ALIGN_CENTER);
                else if (COL == 3) fl_draw("Action", X, Y, W, H, FL_ALIGN_CENTER);
                fl_pop_clip();
                break;

            case CONTEXT_CELL: {
                if (ROW >= (int)wheel_data->games.size()) break;

                fl_push_clip(X, Y, W, H);

                fl_color(FL_WHITE);
                fl_rectf(X, Y, W, H);

                fl_color(FL_LIGHT2);
                fl_rect(X, Y, W, H);

                fl_color(FL_BLACK);
                fl_font(FL_HELVETICA, 14);

                if (COL == 0) {
                    fl_draw(wheel_data->games[ROW].name.c_str(), X + 2, Y, W, H, FL_ALIGN_LEFT);
                } else if (COL == 1) {
                    snprintf(buffer, sizeof(buffer), "%.0f", wheel_data->games[ROW].amount);
                    fl_draw(buffer, X, Y, W, H, FL_ALIGN_CENTER);
                } else if (COL == 2) {
                    double prob = wheel_data->total_amount > 0 ?
                                  100.0 * wheel_data->games[ROW].amount / wheel_data->total_amount : 0;
                    snprintf(buffer, sizeof(buffer), "%.1f%%", prob);
                    fl_draw(buffer, X, Y, W, H, FL_ALIGN_CENTER);
                } else if (COL == 3) {
                    fl_draw("[X]", X, Y, W, H, FL_ALIGN_CENTER);
                }

                fl_pop_clip();
                break;
            }

            default:
                break;
        }
    }

    int handle(int event) override {
        int ret = Fl_Table::handle(event);

        if (event == FL_RELEASE && Fl::event_clicks() == 0) {
            int row = -1, col = -1;
            ResizeFlag resizeflag;
            TableContext context = cursor2rowcol(row, col, resizeflag);

            if (context == CONTEXT_CELL) {
                if (col == 3 && row >= 0 && row < (int)wheel_data->games.size()) {
                    wheel_data->remove_game(row);
                    update_rows();
                    wheel_widget->redraw();
                    return 1;
                } else if ((col == 0 || col == 1) && row >= 0 && row < (int)wheel_data->games.size()) {
                    edit_cell(row, col);
                    return 1;
                }
            }
        }

        return ret;
    }

    void edit_cell(int row, int col) {
        const char* result = nullptr;

        if (col == 0) {
            result = fl_input("Edit game name:", wheel_data->games[row].name.c_str());
            if (result && strlen(result) > 0) {
                wheel_data->games[row].name = result;
            }
        } else if (col == 1) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f", wheel_data->games[row].amount);
            result = fl_input("Edit amount:", buf);
            if (result) {
                try {
                    double amount = std::stod(result);
                    if (amount >= 0) {
                        wheel_data->games[row].amount = amount;
                        wheel_data->recalculate_total();
                    }
                } catch(...) {}
            }
        }

        redraw();
        wheel_widget->redraw();
    }
};

class Task5Window : public Fl_Window {
    WheelOfFortune wheel_data;
    WheelWidget* wheel_widget;
    GamesTable* table;
    Fl_Input* new_game_name;
    Fl_Input* new_game_amount;
    Fl_Button* add_button;
    Fl_Button* spin_button;
    Fl_Box* result_label;
    Fl_Box* winner_label;

public:
    Task5Window() : Fl_Window(900, 650, "Wheel of Fortune") {
        color(fl_rgb_color(167, 191, 167));

        table = new GamesTable(10, 10, 380, 300, &wheel_data, nullptr);

        Fl_Box* new_game_label = new Fl_Box(10, 320, 180, 20, "New game:");
        new_game_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        new_game_name = new Fl_Input(10, 340, 180, 30);

        Fl_Box* amount_label = new Fl_Box(200, 320, 90, 20, "Amount:");
        amount_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        new_game_amount = new Fl_Input(200, 340, 90, 30);
        new_game_amount->value("100");

        add_button = new Fl_Button(300, 340, 90, 30, "Add");
        add_button->callback(add_game_callback, this);

        spin_button = new Fl_Button(10, 380, 380, 60, "SPIN THE WHEEL!");
        spin_button->color(FL_RED);
        spin_button->labelsize(20);
        spin_button->callback(spin_callback, this);

        result_label = new Fl_Box(10, 450, 380, 180, "Click SPIN to start!");
        result_label->box(FL_BORDER_BOX);
        result_label->labelsize(16);
        result_label->align(FL_ALIGN_WRAP | FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

        wheel_widget = new WheelWidget(410, 10, 480, 480, &wheel_data);

        winner_label = new Fl_Box(410, 500, 480, 140, "");
        winner_label->box(FL_BORDER_BOX);
        winner_label->labelsize(20);
        winner_label->labelfont(FL_HELVETICA_BOLD);
        winner_label->align(FL_ALIGN_WRAP | FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

        table->wheel_widget = wheel_widget;

        end();
        resizable(this);
    }

    static void add_game_callback(Fl_Widget* w, void* data) {
        Task5Window* win = (Task5Window*)data;

        const char* name = win->new_game_name->value();
        const char* amount_str = win->new_game_amount->value();

        if (strlen(name) == 0) {
            fl_alert("Please enter game name");
            return;
        }

        double amount = 0;
        try {
            amount = std::stod(amount_str);
            if (amount < 0) throw std::exception();
        } catch(...) {
            fl_alert("Amount must be a non-negative number");
            return;
        }

        win->wheel_data.add_game(name, amount);
        win->table->update_rows();
        win->wheel_widget->redraw();

        win->new_game_name->value("");
        win->new_game_amount->value("100");
    }

    static void spin_callback(Fl_Widget* w, void* data) {
        Task5Window* win = (Task5Window*)data;

        if (win->wheel_data.games.empty()) {
            fl_alert("Add at least one game!");
            return;
        }

        if (win->wheel_data.total_amount <= 0) {
            fl_alert("Total amount must be > 0");
            return;
        }

        int selected = win->wheel_data.spin();

        if (selected >= 0) {
            win->result_label->copy_label("Spinning...");
            win->winner_label->copy_label("");
            win->spin_button->deactivate();
            win->wheel_widget->start_spin(selected);

            Fl::add_timeout(3.0, show_result_callback,
                           new std::pair<Task5Window*, int>(win, selected));
        }
    }

    static void show_result_callback(void* data) {
        auto* pair = (std::pair<Task5Window*, int>*)data;
        Task5Window* win = pair->first;
        int selected = pair->second;

        if (selected >= 0 && selected < (int)win->wheel_data.games.size()) {
            std::string msg = "WINNER:\n\n" + win->wheel_data.games[selected].name +
                             "\n\nAmount: " + std::to_string((int)win->wheel_data.games[selected].amount);
            win->winner_label->copy_label(msg.c_str());
            win->result_label->copy_label("Spin complete!");
        }

        win->spin_button->activate();
        delete pair;
    }
};

namespace task5 {
    static void show_window(Fl_Widget*, void*) {
        Task5Window* win = new Task5Window();
        win->show();
    }
}
