#include "ui.h"
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        auto app = Gtk::Application::create(argc, argv, "org.example.lab2");

        auto builder = Gtk::Builder::create_from_file("ui.glade");
        ui* window = nullptr;
        builder->get_widget_derived("main_window", window);

        if(!window) {
            std::cerr << "Can`t load 'main_window' from .glade fil" << std::endl;
            return 1;
        }

        return app->run(*window);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}