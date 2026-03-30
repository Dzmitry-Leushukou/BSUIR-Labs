#include <iostream>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) {
        std::cout << "Error: division by zero!" << std::endl;
        return 0;
    }
    return a / b;
}

int main() {
    int choice, x, y;
    std::cout << "Simple Calculator\n";
    std::cout << "1. Add\n2. Subtract\n3. Multiply\n4. Divide\n";
    std::cout << "Enter choice: ";
    std::cin >> choice;

    std::cout << "Enter two numbers: ";
    std::cin >> x >> y;

    int result = 0;
    switch (choice) {
        case 1: result = add(x, y); break;
        case 2: result = subtract(x, y); break;
        case 3: result = multiply(x, y); break;
        case 4: result = divide(x, y); break;
        default: std::cout << "Invalid choice\n"; return 1;
    }

    std::cout << "Result: " << result << std::endl;
    return 0;
}