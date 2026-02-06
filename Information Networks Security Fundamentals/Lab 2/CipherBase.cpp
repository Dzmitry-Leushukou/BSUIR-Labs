#include "CipherBase.h"
#include <filesystem>
#include <locale>
#include <codecvt>

std::vector<std::wstring> CipherBase::ReadFile(const std::wstring& filepath) {
    std::filesystem::path input_filepath = filepath;
    std::wifstream fin(input_filepath);
    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8<wchar_t>));
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
    fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf8<wchar_t>));

    for (const auto& s : text) {
        fout << s<<L'\n';
    }
    fout.close();
}

wchar_t CipherBase::encryptSymbol(const wchar_t& symbol, long long shift) {
    shift = shift % 0x10000;
    if (shift < 0) {
        shift += 0x10000;
    }

    wchar_t encrypted = symbol + static_cast<wchar_t>(shift);

    if (encrypted > 0xFFFF) {
        encrypted -= 0x10000;
    }
    return encrypted;
}