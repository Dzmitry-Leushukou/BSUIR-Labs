#include "JpegSteganography.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <windows.h>
#include <shellapi.h>

int main() {
    try {
        std::vector<std::string> args;
        int count = 0;
        auto wide = CommandLineToArgvW(GetCommandLineW(), &count);
        if (!wide) throw std::runtime_error("Cannot read command line.");
        for (int i = 1; i < count; ++i) {
            const int size = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
            std::string value(size, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, value.data(), size, nullptr, nullptr);
            value.pop_back();
            args.push_back(value);
        }
        LocalFree(wide);
        SetConsoleOutputCP(CP_UTF8);
        if (args.size() == 1) args.insert(args.begin(), "info");
        if (args.size() < 2 ||
            !((args[0] == "info" && args.size() == 2) ||
              (args[0] == "extract" && (args.size() == 2 || args.size() == 3)) ||
              ((args[0] == "hide" || args[0] == "hide-file") && args.size() == 4))) {
            std::cerr << "Usage:\n"
                      << "  Lab8 info image.jpg\n"
                      << "  Lab8 image.jpg\n"
                      << "  Lab8 hide image.jpg \"message\" stego.jpg\n"
                      << "  Lab8 hide-file image.jpg message.txt stego.jpg\n"
                      << "  Lab8 extract stego.jpg [message.txt]\n";
            return 1;
        }

        JpegSteganography image(std::filesystem::u8path(args[1]));
        if (args[0] == "info") {
            const auto bytes = image.capacity();
            std::cout << "Container: " << image.availableBits() << " bits.\n"
                      << "Capacity: " << bytes << " message bytes (" << bytes * 8 << " bits)";
            if (bytes >= 1024) {
                const bool megabytes = bytes >= 1024 * 1024;
                std::cout << " = " << std::fixed << std::setprecision(2)
                          << double(bytes) / (megabytes ? 1024 * 1024 : 1024)
                          << (megabytes ? " MiB" : " KiB");
            }
            std::cout << ". Header overhead included.\n";
        } else if (args[0] == "hide" || args[0] == "hide-file") {
            std::string text = args[2];
            if (args[0] == "hide-file") {
                std::ifstream input(std::filesystem::u8path(args[2]), std::ios::binary);
                if (!input) throw std::runtime_error("Cannot open message file.");
                text.assign(std::istreambuf_iterator<char>(input), {});
                if (input.bad()) throw std::runtime_error("Cannot read message file.");
            }
            image.hide(text, std::filesystem::u8path(args[3]));
            std::cout << "Hidden " << text.size() << " bytes.\n";
        } else {
            const auto text = image.extract();
            if (args.size() == 3) {
                const auto output = std::filesystem::u8path(args[2]);
                if (std::filesystem::exists(output) &&
                    std::filesystem::equivalent(std::filesystem::u8path(args[1]), output))
                    throw std::runtime_error("Do not overwrite the input image with text.");
                std::ofstream file(output, std::ios::binary);
                if (!file) throw std::runtime_error("Cannot open output text file.");
                file.write(text.data(), static_cast<std::streamsize>(text.size()));
                file.close();
                if (!file) throw std::runtime_error("Cannot write output text file.");
                std::cout << "Extracted " << text.size() << " bytes.\n";
            } else {
                std::cout << text << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
