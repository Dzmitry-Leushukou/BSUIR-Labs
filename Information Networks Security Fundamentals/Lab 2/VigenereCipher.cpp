//
// Created by dzmitry-leushukou on 2/2/26.
//

#include "VigenereCipher.h"



void VigenereCipher::encrypt(const std::wstring& key, const std::wstring& filepath) {
    std::vector<std::wstring> text=ReadFile(filepath);
    std::vector<std::wstring> encrypted_text;

    unsigned long long  index=0;
    for (auto& i:text) {
        std::wstring s=L"";
        encrypted_text.push_back(s);
        for (auto& j:i) {
            if (index == key.size())
                index=0;
            encrypted_text.back().push_back(encryptSymbol(j, key.at(index)));
        }

    }

    SaveFile(filepath, encrypted_text);
}

void VigenereCipher::decrypt(const std::wstring& key, const std::wstring& filepath)  {
    std::wstring reversed_key;
    for (auto& i:key) {
        reversed_key.push_back(i*(-1));

    }
    encrypt(reversed_key, filepath);
}