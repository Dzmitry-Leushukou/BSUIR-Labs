#include <iostream>
#include "UI.h"

int main()
{
    std::ios_base::sync_with_stdio(0);
    std::cin.tie(0);
    UI* ui = new UI();
    ui->inputHandler();
}
