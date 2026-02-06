#include "ui.h"
#include <iostream>

ui::ui(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject), builder(builder) {

    cc = new CaesarCipher();
    vc = new VigenereCipher();

    green.set_rgba(0.0, 1.0, 0.0, 1.0);
    red.set_rgba(1.0, 0.0, 0.0, 1.0);
    builder->get_widget("ChooseFileButton", chooseFileButton);
    builder->get_widget("filepath", filepath);
    builder->get_widget("vigenereEncryptionButton", vigenereEncryptionButton);
    builder->get_widget("vigenereDecryptionButton", vigenereDecryptionButton);
    builder->get_widget("caesarEncryptionButton", caesarEncryptionButton);
    builder->get_widget("caesarDecryptionButton", caesarDecryptionButton);
    builder->get_widget("filepath", filepath);
    builder->get_widget("statusLabel", statusLabel);
    builder->get_widget("keyEntry", keyEntry);

    if(chooseFileButton) {
        chooseFileButton->signal_clicked().connect(
            sigc::mem_fun(*this, &ui::on_ChooseFileButton_clicked)
        );
    }

    if (caesarEncryptionButton) {
        caesarEncryptionButton->signal_clicked().connect(
            sigc::mem_fun(*this,&ui::on_caesarEncryptionButton_clicked));
    }

    if (caesarDecryptionButton) {
        caesarDecryptionButton->signal_clicked().connect(
            sigc::mem_fun(*this,&ui::on_caesarDecryptionButton_clicked));
    }

    if (vigenereDecryptionButton) {
        vigenereDecryptionButton->signal_clicked().connect(
            sigc::mem_fun(*this,&ui::on_vigenereDecryptionButton_clicked));
    }

    if (vigenereEncryptionButton) {
        vigenereEncryptionButton->signal_clicked().connect(
            sigc::mem_fun(*this,&ui::on_vigenereEncryptionButton_clicked));
    }

}

void ui::on_ChooseFileButton_clicked() {
    Gtk::FileChooserDialog dialog("Выберите файл", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    dialog.add_button("_Отмена", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Открыть", Gtk::RESPONSE_OK);

    if(dialog.run() == Gtk::RESPONSE_OK) {
        filepath->set_text(dialog.get_filename());
    }

}

void ui::on_caesarEncryptionButton_clicked() {
    if (filepath->get_text().length() == 0) {
        update_status("Before making operation choose file",false);
        return;
    }
    if (keyEntry->get_text().length() == 0) {
        update_status("Key place can not be empty",false);
        return;
    }
    try {
        cc->encrypt( getShift(keyEntry->get_text()),filepath->get_text());
    }
    catch (std::exception& e) {
        update_status(e.what(),false);
        return;
    }
    update_status("Success!");
}

void ui::on_caesarDecryptionButton_clicked() {
    if (filepath->get_text().length() == 0) {
        update_status("Before making operation choose file",false);
        return;
    }
    if (keyEntry->get_text().length() == 0) {
        update_status("Key place can not be empty",false);
        return;
    }
    try {
        cc->decrypt( getShift(keyEntry->get_text()),filepath->get_text());
    }
    catch (std::exception& e) {
        update_status(e.what(),false);
        return;
    }
    update_status("Success!");
}

void ui::on_vigenereEncryptionButton_clicked() {
    if (filepath->get_text().length() == 0) {
        update_status("Before making operation choose file",false);
        return;
    }
    if (keyEntry->get_text().length() == 0) {
        update_status("Key place can not be empty",false);
        return;
    }
    try {
        vc->encrypt( keyEntry->get_text(),filepath->get_text());
    }
    catch (std::exception& e) {
        update_status(e.what(),false);
        return;
    }
    update_status("Success!");
}

void ui::on_vigenereDecryptionButton_clicked() {
    if (filepath->get_text().length() == 0) {
        update_status("Before making operation choose file",false);
        return;
    }
    if (keyEntry->get_text().length() == 0) {
        update_status("Key place can not be empty",false);
        return;
    }
    try {
        vc->decrypt( keyEntry->get_text(),filepath->get_text());
    }
    catch (std::exception& e) {
        update_status(e.what(),false);
        return;
    }
    update_status("Success!");
}


void ui::update_status(const std::string& message, bool response_ok) {
    statusLabel->set_text(message);
    if (response_ok) {
        statusLabel->override_color(green);
        return;
    }
    statusLabel->override_color(red);
}

long long ui::getShift(const std::string& key) {
    for (auto& i:key) {
        if (i<'0' || i>'9') {
            throw std::runtime_error("Shift can`t contain any chars except numbers");
        }
    }
    long long shift = 0;
    try {
        shift=std::stoll(key);
    }
    catch (std::exception& e) {
        throw std::runtime_error("Bad key");
    }

    return shift;
}