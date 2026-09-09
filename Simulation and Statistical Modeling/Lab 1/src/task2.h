#pragma once

#include <thread>
#include <string>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Widget.H>


#include "utils.h"
#include "task1.h"

#include <iostream>

class task2
{
    public:
        static std::vector<long long> run(const char* input, long long n = 1)
        {
            std::vector<double> p = processInput(input);
            std::vector<long long>result;
            for(auto& i : p)
            {
                result.push_back(task1::runMultiple(i, n));
            }

            return result;
            
        }

        static void callback(Fl_Widget* button, void* data)
        {
            button->deactivate();

            task2DTO* tmp = static_cast<task2DTO*>(data);

            std::string input = tmp->in->value();
            long long n = tmp->n;

            Fl_Multiline_Output* out = tmp->out;

            std::thread calc(
                [input, n, button, out]()
                {
                    std::vector<long long> v = run(input.c_str(), n);
                    std::string* text_result = new std::string(to_str(v,n));

                    Fl::awake(
                        [](void* userdata)
                        {
                            auto params =
                                static_cast<
                                    std::pair<std::pair<Fl_Widget*, Fl_Multiline_Output*>,std::string*>*
                                    >(userdata);

                            Fl_Widget* btn = params->first.first;
                            Fl_Multiline_Output* out = params->first.second;
                            std::string* str = params->second;

                            out->value(str->c_str());
                            btn->activate();
                            Fl::redraw();

                            delete str;
                            delete params;
                        },
                        new std::pair<
                            std::pair<Fl_Widget*, Fl_Multiline_Output*>,
                            std::string*
                        >(
                            std::make_pair(button, out),
                            text_result
                        )
                    );
                }
            );

            calc.detach();
        }

        static std::string to_str(std::vector<long long> v, long long n)
        {
            std::string res;
            for(auto& i:v)
            {
                double tmp = i;
                tmp/=(double)n;
                res += std::to_string(tmp) + "\n";
            }
            if(!res.empty())
                res.pop_back();
            
             return res;
        }

        struct task2DTO
        {
            Fl_Multiline_Input* in;
            Fl_Multiline_Output* out;
            long long n;

        };
    
    private:
        static std::vector <double> processInput(const char* input)
        {
            std::vector <double> res;
            std::string text(input);
            std::string tmp;
            for(auto& i:text)
            {
                if(i == '\n')
                {
                    try{
                    res.push_back(std::stod(tmp));
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "Error: " << e.what() << '\n';
                    }
                    tmp = "";
                    continue;
                }
    
                tmp += i;
            }
            if (tmp != "")
                res.push_back(std::stod(tmp));
            return res;
        }
};
