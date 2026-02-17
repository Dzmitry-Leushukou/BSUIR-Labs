#ifndef UI_H
#define UI_H

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/builder.h>
#include <gtkmm/filechooserdialog.h>
#include <gdkmm/rgba.h>
#include <filesystem>

#include "VigenereCipher.h"
#include "CaesarCipher.h"


class ui : public Gtk::Window {
public:
    ui(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~ui() = default;

private:
    //ui parts
    Gtk::Button* chooseFileButton = nullptr;
    Gtk::Button* caesarEncryptionButton=nullptr;
    Gtk::Button* caesarDecryptionButton=nullptr;
    Gtk::Button* vigenereEncryptionButton=nullptr;
    Gtk::Button* vigenereDecryptionButton=nullptr;
    Gtk::Entry* keyEntry=nullptr;
    Gtk::Label* statusLabel=nullptr;
    Glib::RefPtr<Gtk::Builder> builder;
    Gtk::Label* filepath=nullptr;
    Gdk::RGBA green,red;

    //signals
    void on_ChooseFileButton_clicked();
    void on_caesarEncryptionButton_clicked();
    void on_caesarDecryptionButton_clicked();
    void on_vigenereEncryptionButton_clicked();
    void on_vigenereDecryptionButton_clicked();

    //methods
    void update_status(const std::string&,bool response_ok = true);
    long long getShift(const std::string& key);
    //fields
    CaesarCipher * cc=nullptr;
    VigenereCipher * vc=nullptr;
};

#endif // UI_H