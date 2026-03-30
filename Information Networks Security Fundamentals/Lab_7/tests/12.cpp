#include <iostream>
#include <string>

bool x0(const std::string& x1) {
    std::string x2 = "secret123";
    return x1 == x2;
}

void x3() {
    std::cout << "Welcome to the secure system!" << std::endl;
}

int main() {
    x3();
    std::string x4;
    std::cout << "Enter password: ";
    std::cin >> x4;

    if (x0(x4)) {
        std::cout << "Access granted. You are now logged in." << std::endl;
    } else {
        std::cout << "Access denied. Wrong password." << std::endl;
    }

    return 0;
}