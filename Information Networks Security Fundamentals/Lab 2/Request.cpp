//
// Created by dzmitry-leushukou on 2/3/26.
//

#include "Request.h"

#include <iostream>
#include <ostream>
#include <stdexcept>

Request::Request(std::wstring raw) {
        try {
            std::wstring filepath;
            std::wstring flag=divide_for_nearest_space(raw);
            raw=raw.substr(flag.size()+1,raw.length());

            std::wstring shift=divide_for_nearest_space(raw);

            filepath=raw.substr(shift.size()+1,raw.length());



            if (filepath.size()==0 || shift.size()==0 || flag.size()!=1)
                throw std::runtime_error("Can`t divide string");


            this->flag=flag.at(0);
            if (this->flag==L'c' || this->flag==L's')
            for (int i=0;i<shift.size();i++) {
                if (shift.at(i)>='0' && shift.at(i)<='9') {
                    //good
                }
                else
                    if (shift.at(i)!='-')
                        throw std::runtime_error("Incorrect shift format");
                    else
                        if (i!=0)
                            throw std::runtime_error("Incorrect shift format");
            }

            else {
                this->key=shift;
            }
            if (!checkFile(filepath))
                throw std::runtime_error("Incorrect filepath format");

            if (flag.size()==1 && flag.at(0) != L'c' && flag.at(0) != L's' && flag.at(0) != L'e' && flag.at(0) != L'd')
                throw std::runtime_error("Incorrect flag");
            //try to cast
            this->filepath=filepath;

            this->shift=std::stoll(shift);

        }
    catch (std::exception& e) {
        std::cerr << "[REQUEST] "<<e.what() << std::endl;
        throw std::runtime_error("Can`t create new entity");
    }
}

std::wstring Request::divide_for_nearest_space(const std::wstring& raw) {
    std::wstring r;
    for (auto& i:raw) {
        if (i==' ') {
            return r;
        }
        r+=i;

    }
    throw std::runtime_error("Can`t divide string");
}

char Request::getFlag() const {
    return this->flag;
}
long long Request::getShift() const {
    return this->shift;
}
std::wstring Request::getFilepath() const {
    return this->filepath;
}
std::wstring Request::getKey() const {
    return this->key;
}

bool Request::checkFile(const std::wstring& filepath) {
    std::filesystem::path file(filepath);
    if (filepath.length() < 1)
        return false;

    if (!std::filesystem::exists(file)) {
        return false;
    }

    if (!std::filesystem::is_regular_file(file)) {
        return false;
    }

    std::wifstream fin(file);
    if (!fin.is_open()) {
        return false;
    }


    return true;

}
