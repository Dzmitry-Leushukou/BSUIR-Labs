//
// Created by dzmitry-leushukou on 2/2/26.
//

#ifndef LAB_2_CAESARCIPHER_H
#define LAB_2_CAESARCIPHER_H


#include "CipherBase.h"
#include <vector>
#include <string>


class CaesarCipher:public CipherBase{
    public:
    void encrypt(long long shift, const std::wstring& filepath) override;
    void decrypt(long long shift, const std::wstring& filepath) override;



};


#endif //LAB_2_CAESARCIPHER_H