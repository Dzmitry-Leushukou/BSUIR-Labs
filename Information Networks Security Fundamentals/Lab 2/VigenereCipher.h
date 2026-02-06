//
// Created by dzmitry-leushukou on 2/2/26.
//

#ifndef LAB_2_VIGENERECIPHER_H
#define LAB_2_VIGENERECIPHER_H

#include "CipherBase.h"

class VigenereCipher: public CipherBase {
public:
    void encrypt(const std::string& key, const std::string& filepath);
    void decrypt(const std::string& key, const std::string& filepath);

};


#endif //LAB_2_VIGENERECIPHER_H