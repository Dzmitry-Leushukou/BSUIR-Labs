#pragma once
#include "FileSaver.h"
#include <nlohmann/json.hpp>
class JSONSaver :
    public FileSaver
{
    void save(std::vector<std::string> content, std::string filepath) override;
};

