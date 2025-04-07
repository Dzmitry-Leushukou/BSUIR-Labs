#pragma once
#include "FileSaver.h"
class TXTSaver :
    public FileSaver
{
    void save(std::vector<std::string> content, std::string filepath) override;
};

