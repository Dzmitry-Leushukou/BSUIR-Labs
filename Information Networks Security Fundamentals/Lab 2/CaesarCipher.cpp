//
// Created by dzmitry-leushukou on 2/2/26.
//

#include "CaesarCipher.h"

void CaesarCipher::encrypt(long long shift, const std::wstring& filepath) {
    std::vector<std::wstring> text=ReadFile(filepath);
    std::vector<std::wstring> encrypted_text;

    for (auto& i:text) {
        std::wstring s=L"";
        encrypted_text.push_back(s);
        for (auto& j:i) {
            encrypted_text.back().push_back(encryptSymbol(j, shift));
        }

    }

    SaveFile(filepath, encrypted_text);
}

void CaesarCipher::decrypt(long long shift, const std::wstring& filepath) {
    encrypt(shift*(-1), filepath);
}