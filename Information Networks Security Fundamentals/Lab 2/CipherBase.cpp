//
// Created by dzmitry-leushukou on 2/2/26.
//

#include "CipherBase.h"

#include <filesystem>


std::vector<std::string> CipherBase::ReadFile(const std::string& filepath) {
    std::filesystem::path input_filepath = filepath;
    std::ifstream fin(input_filepath);
    std::string s;
    std::vector<std::string> text;
    while (std::getline(fin,s)) {
        text.push_back(s);
    }
    fin.close();
    return text;
}

void CipherBase::SaveFile(const std::string& filepath, const std::vector<std::string>& text) {
    std::filesystem::path input_filepath = filepath;
    std::ofstream fout(input_filepath);

    for (const auto& s : text) {
        fout << s<<'\n';
    }
    fout.close();
}

char CipherBase::encryptSymbol(const char& symbol, long long shift) {
    shift = shift % 0x10000;
    if (shift < 0) {
        shift += 0x10000;
    }

    char encrypted = symbol + static_cast<wchar_t>(shift);

    if (encrypted > 0xFFFF) {
        encrypted -= 0x10000;
    }
    return encrypted;
}