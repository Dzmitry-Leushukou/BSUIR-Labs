#include <iostream>
#include <string>

bool checkPassword(const std::string& input) {
    std::string secret = "secret123";
    return input == secret;
}

void welcomeMessage() {
    std::cout << "Welcome to the secure system!" << std::endl;
}

int main() {
    welcomeMessage();
    std::string password;
    std::cout << "Enter password: ";
    std::cin >> password;

    if (checkPassword(password)) {
        std::cout << "Access granted. You are now logged in." << std::endl;
    } else {
        std::cout << "Access denied. Wrong password." << std::endl;
    }

    return 0;
}