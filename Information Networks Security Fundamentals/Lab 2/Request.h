//
// Created by dzmitry-leushukou on 2/3/26.
//

#ifndef LAB_2_REQUEST_H
#define LAB_2_REQUEST_H
#include <string>
#include <filesystem>
#include <fstream>

class Request {
    public:
    Request(std::wstring raw);

    char getFlag() const;
    long long getShift() const;
    std::wstring getFilepath() const;
    std::wstring getKey() const;
    private:
    char flag;
    long long shift;
    std::wstring filepath,key;

    std::wstring divide_for_nearest_space(const std::wstring& raw);
    bool checkFile(const std::wstring& filepath);
};


#endif //LAB_2_REQUEST_H