#pragma once
#include "FileSaver.h"
#include "tinyxml2.h"
class XMLSaver :
    public FileSaver
{
    void save(std::vector<std::string> content, std::string filepath) override;
};

