#pragma once

#include <random>
#include <cmath>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <thread>
#include <atomic>

#include <iostream>

#include "config.h"

class task1
{
private:
    inline static std::mt19937_64 gen{std::random_device{}()};  
    inline static std::uniform_real_distribution<double> dist{0.0, 1.0};

public:
    static bool runSingle(double p)
    {
        return dist(gen) < p;
    }

    static long long runMultiple(double p, long long n)
    {
        int threadsNumber = 1 + (log10(n) / 2.0);
        std::vector <std::thread> threads;
        std::atomic_llong result = 0;

        long long chunk = n / threadsNumber;
        long long last = n % threadsNumber;

        for(int i = 0; i < threadsNumber; i++)
        {
            if (i == threadsNumber - 1)
                chunk += last;

            threads.push_back(std::thread([&result, p, chunk](){
                long long local = 0;
                for(long long i = 0; i < chunk; i++)
                {
                    local += runSingle(p);
                }

                result += local;
            }));
        }

        for(auto& i : threads)
        {
            i.join();
        }

        return result.load();
    }

    static void simulate_callback(Fl_Widget* button, void* data)
    {
        task1::SingleDTO* tmp = static_cast<task1::SingleDTO*>(data);
        double val = std::stod(tmp->input->value());
        bool result = task1::runSingle(val);
        tmp->aim->label(result ? "True" : "False");
        tmp->aim->redraw();
    }

    static void simulate_multiple_callback(Fl_Widget* button, void* data)
    {
        task1::MultipleDTO* tmp = static_cast <task1::MultipleDTO*> (data);
        double p = std::stod(tmp -> P -> value());
        long long n = std::stoll(tmp -> N -> value());
        button -> deactivate();
        Fl_Box* aim = tmp -> aim;
        
        std::thread return_result([button, aim, n, p](){
            long long result = task1::runMultiple(p, n);
            std::string* text_result = new std::string(SIMULATE_MULTIPLE_TASK_RESULT_TEXT);
            *text_result += std::to_string((long double)result / (long double)(n));
            
            Fl::awake([](void* userdata) {
                auto params = static_cast<std::pair<std::pair<Fl_Widget*, Fl_Box*>, std::string*>*>(userdata);
                
                Fl_Widget* btn = params->first.first;
                Fl_Box* box = params->first.second;
                std::string* str = params->second;

                box->copy_label(str->c_str()); 
                btn->activate();        
                
                Fl::redraw();          

                delete str;
                delete params;
            }, new std::pair<std::pair<Fl_Widget*, Fl_Box*>, std::string*>({button, aim}, text_result));
        });

        return_result.detach();
    }


    struct SingleDTO{
        Fl_Input* input;
        Fl_Box* aim;
    };

    struct MultipleDTO{
        Fl_Input* N;
        Fl_Input* P;
        Fl_Box* aim;
    };
};
