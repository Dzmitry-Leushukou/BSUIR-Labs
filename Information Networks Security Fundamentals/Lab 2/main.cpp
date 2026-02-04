#include <filesystem>
#include <iostream>
#include <fstream>
#include <limits.h>

#include "CaesarCipher.h"
#include "Request.h"
#include "VigenereCipher.h"


CaesarCipher* caesarCipher=nullptr;
VigenereCipher* vigenereCipher=nullptr;


void help() {
    std::cout<<std::endl<<"======================="<<std::endl<<"Request build by the next structure:"<<std::endl<<"flag shift filepath"<<std::endl<<
        "flag: char (string with size 1)"<<std::endl<<
            "\tc - Caesar encryption"<<std::endl<<
            "\ts - Caesar decryption"<<std::endl<<
            "\te - Vigenere encryption"<<std::endl<<
            "\td - Vigenere decryption"<<std::endl<<
        "shift: integer into ["<<LLONG_MIN<<"; "<<LLONG_MAX<<"] for Caesar"<<std::endl<<
        "       string (max_size should be <= size of text file data. Otherwise, program use first {size of text file data} chars."<<std::endl<<
        "filepath: filepath to the aim text file"<<std::endl<<
        "F show this menu again type \"help\"  \"/help\""<< std::endl<<
        "======================="<<std::endl<<std::endl;
}

void process_request(const Request& request) {
    switch (request.getFlag()) {
        case L'c' : {
            caesarCipher->encrypt(request.getShift(),request.getFilepath());
            break;
        }

        case 's' : {
            caesarCipher->decrypt(request.getShift(),request.getFilepath());
            break;
        }
        case 'e' :
            vigenereCipher->encrypt(request.getKey(),request.getFilepath());
            break;
        case 'd' :
            vigenereCipher->decrypt(request.getKey(),request.getFilepath());
            break;
    }

}

int main() {
    std::wstring req;
    std::locale::global(std::locale(""));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());

    vigenereCipher=new VigenereCipher();
    caesarCipher=new CaesarCipher();

    Request * request = nullptr;

    help();
    while (std::getline(std::wcin,req)){
        try {
            if (req==L"/help" or  req==L"help") {
                help();
                continue;
            }
            request = new Request(req);

        }catch (std::exception& e) {
            delete request;
            request = nullptr;
            std::cerr << e.what() << std::endl;
            // return 0;
            continue;
        }

        std::cout<<"Success!"<<std::endl;
        process_request(*request);

        delete request;
        request = nullptr;
        // return 0;

    }


    delete request;
    request = nullptr;
    delete caesarCipher;
    delete vigenereCipher;
    caesarCipher=nullptr;
    vigenereCipher=nullptr;
    return 0;

}
