#pragma once
#include "FileSaver.h"
class MDSaver :
    public FileSaver
{
    void save(std::vector<std::string> content, std::string filepath) override;
};

