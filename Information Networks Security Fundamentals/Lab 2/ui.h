#ifndef UI_H
#define UI_H

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/builder.h>
#include <gtkmm/filechooserdialog.h>
#include <filesystem>

class ui : public Gtk::Window {
public:
    ui(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~ui() = default;

private:
    Gtk::Button* chooseFileButton = nullptr;
    Gtk::Button* caesarEncryptionButton=nullptr;
    Gtk::Button* caesarDecryptionButton=nullptr;
    Gtk::Button* vigenereEncryptionButton=nullptr;
    Gtk::Button* vigenereDecryptionButton=nullptr;

    Gtk::Label* filepath=nullptr;
    Glib::RefPtr<Gtk::Builder> builder;

    //signals
    void on_ChooseFileButton_clicked();




};

#endif // UI_H