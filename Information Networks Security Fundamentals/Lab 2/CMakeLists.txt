cmake_minimum_required(VERSION 4.1)
project(Lab_2)

set(CMAKE_CXX_STANDARD 20)

add_executable(Lab_2 main.cpp
        CaesarCipher.cpp
        CaesarCipher.h
        VigenereCipher.cpp
        VigenereCipher.h
        CipherBase.cpp
        CipherBase.h
        Request.cpp
        Request.h)
