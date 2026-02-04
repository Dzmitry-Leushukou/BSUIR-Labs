//
// Created by dzmitry-leushukou on 2/2/26.
//

#include "CipherBase.h"

#include <filesystem>


std::vector<std::wstring> CipherBase::ReadFile(const std::wstring& filepath) {
    std::filesystem::path input_filepath = filepath;
    std::wifstream fin(input_filepath);
    std::wstring s;
    std::vector<std::wstring> text;
    while (std::getline(fin,s)) {
        text.push_back(s);
    }
    fin.close();
    return text;
}

void CipherBase::SaveFile(const std::wstring& filepath, const std::vector<std::wstring>& text) {
    std::filesystem::path input_filepath = filepath;
    std::wofstream fout(input_filepath);

    for (const auto& s : text) {
        fout << s<<L'\n';
    }
    fout.close();
}

wchar_t CipherBase::encryptSymbol(const wchar_t& symbol, long long shift) {
    shift = shift % 0x10000;
    if (shift < 0) {
        shift += 0x10000; // Обрабатываем отрицательные сдвиги
    }

    wchar_t encrypted = symbol + static_cast<wchar_t>(shift);

    // Проверяем, не вышли ли мы за пределы BMP (Basic Multilingual Plane)
    if (encrypted > 0xFFFF) {
        encrypted -= 0x10000;
    }
    return encrypted;
}