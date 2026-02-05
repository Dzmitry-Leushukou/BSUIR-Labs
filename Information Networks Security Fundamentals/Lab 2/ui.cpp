#include "ui.h"
#include <iostream>

ui::ui(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject), builder(builder) {

    builder->get_widget("ChooseFileButton", chooseFileButton);
    builder->get_widget("filepath", filepath);
    builder->get_widget("vigenereEncryptionButton", vigenereEncryptionButton);
    builder->get_widget("vigenereDecryptionButton", vigenereDecryptionButton);
    builder->get_widget("caesarEncryptionButton", caesarEncryptionButton);
    builder->get_widget("caesarDecryptionButton", caesarDecryptionButton);

    if(chooseFileButton) {
        chooseFileButton->signal_clicked().connect(
            sigc::mem_fun(*this, &ui::on_ChooseFileButton_clicked)
        );
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
