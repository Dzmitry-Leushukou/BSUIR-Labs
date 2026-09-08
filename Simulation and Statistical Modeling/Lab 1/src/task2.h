#pragma once

#include <thread>
#include <string>

#include "utils.h"

class task2
{
    public:
        static void run(const char* input, int n = 1)
        {
            std::vector<double> p = processInput(input);
            std::vector<std::thread> threads(threadsCount(n));
            
        }

    private:
        static std::vector<double> processInput(const char* input)
        {
            std::vector<double>res;
            std::string text(input);
            std::string tmp;
            for(auto& i:text)
            {
                if(i == '\n')
                {
                    res.push_back(std::stod(tmp));
                    tmp="";
                    continue;
                }
                tmp+=i;
            }
            if (tmp != "")
                res.push_back(std::stod(tmp));
            return res;
        }
};
