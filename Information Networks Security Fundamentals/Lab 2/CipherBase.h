//
// Created by dzmitry-leushukou on 2/2/26.
//

#ifndef LAB_2_CIPHERBASE_H
#define LAB_2_CIPHERBASE_H
#include <string>
#include <vector>
#include <fstream>

class CipherBase {
    public:
    virtual ~CipherBase() = default;
    virtual void encrypt(long long shift, const std::string& filepath){};
    virtual void decrypt(long long shift, const std::string& filepath){};
    char encryptSymbol(const char& symbol, long long shift);
    protected:

    std::vector<std::string> ReadFile(const std::string& filepath);
    void SaveFile(const std::string& filepath, const std::vector<std::string>& text);

};


#endif //LAB_2_CIPHERBASE_H