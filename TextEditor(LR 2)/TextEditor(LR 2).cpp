#include <iostream>
#include "UI.h"

int main()
{
    UI* ui = new UI();
    ui->inputHandler();
    delete ui;
    ui = nullptr;
}
