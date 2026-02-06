//
// Created by dzmitry-leushukou on 2/2/26.
//

#include "VigenereCipher.h"



void VigenereCipher::encrypt(const std::string& key, const std::string& filepath) {
    std::vector<std::string> text=ReadFile(filepath);
    std::vector<std::string> encrypted_text;

    unsigned long long  index=0;
    for (auto& i:text) {
        std::string s="";
        encrypted_text.push_back(s);
        for (auto& j:i) {
            if (index == key.size())
                index=0;
            encrypted_text.back().push_back(encryptSymbol(j, key.at(index)));
        }

    }

    SaveFile(filepath, encrypted_text);
}

void VigenereCipher::decrypt(const std::string& key, const std::string& filepath)  {
    std::string reversed_key;
    for (auto& i:key) {
        reversed_key.push_back(i*(-1));

    }
    encrypt(reversed_key, filepath);
}