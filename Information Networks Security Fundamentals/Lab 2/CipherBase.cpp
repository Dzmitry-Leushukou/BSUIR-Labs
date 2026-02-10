#include "CipherBase.h"
#include <filesystem>
#include <locale>
#include <codecvt>
#include <iostream>

std::vector<std::wstring> CipherBase::ReadFile(const std::wstring& filepath) {
    std::filesystem::path input_filepath = filepath;
    std::wifstream fin(input_filepath, std::ios::binary);

    fin.imbue(std::locale(fin.getloc(),
        new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file for reading");
    }

    std::wstring s;
    std::vector<std::wstring> text;

    while (std::getline(fin, s)) {
        text.push_back(s);
    }

    fin.close();
    return text;
}

void CipherBase::SaveFile(const std::wstring& filepath, const std::vector<std::wstring>& text) {
    std::filesystem::path input_filepath = filepath;
    std::wofstream fout(input_filepath, std::ios::binary);

    fout.imbue(std::locale(fout.getloc(),
        new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));

    if (!fout.is_open()) {
        throw std::runtime_error("Cannot open file for writing");
    }

    for (size_t i = 0; i < text.size(); ++i) {
        fout << text[i];
        if (i < text.size() - 1) {
            fout << L'\n';
        }
    }

    fout.close();
}

wchar_t CipherBase::encryptSymbol(const wchar_t& symbol, long long shift) {
    constexpr auto MAX_WCHAR = static_cast<long long>(std::numeric_limits<wchar_t>::max());
    constexpr auto MIN_WCHAR = static_cast<long long>(std::numeric_limits<wchar_t>::min());

    long long range = MAX_WCHAR - MIN_WCHAR + 1;
    shift = ((shift % range) + range) % range;

    long long result = static_cast<long long>(symbol) + shift;

    if (result > MAX_WCHAR) {
        result -= range;
    } else if (result < MIN_WCHAR) {
        result += range;
    }

    return static_cast<wchar_t>(result);
}