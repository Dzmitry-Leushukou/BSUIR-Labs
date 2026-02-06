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
    virtual void encrypt(long long shift, const std::wstring& filepath){};
    virtual void decrypt(long long shift, const std::wstring& filepath){};
    wchar_t encryptSymbol(const wchar_t& symbol, long long shift);
    protected:

    std::vector<std::wstring> ReadFile(const std::wstring& filepath);
    void SaveFile(const std::wstring& filepath, const std::vector<std::wstring>& text);

};


#endif //LAB_2_CIPHERBASE_H