#pragma once
#include "File.h"
class MD :
    public File
{
public:
    MD(std::string filepath):File(filepath){}
    void preview();

private:

};

