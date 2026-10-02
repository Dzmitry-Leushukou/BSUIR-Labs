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

        static std::vector<std::pair<std::string, double>> calculateOutcomes(const std::vector<double>& probabilities)
        {
            std::vector<std::pair<std::string, double>> outcomes;
            int n = probabilities.size();
            int totalOutcomes = 1 << n;

            for(int mask = 0; mask < totalOutcomes; mask++)
            {
                std::string outcome;
                double probability = 1.0;

                for(int i = 0; i < n; i++)
                {
                    if(mask & (1 << i))
                    {
                        outcome += "T";
                        probability *= probabilities[i];
                    }
                    else
                    {
                        outcome += "F";
                        probability *= (1.0 - probabilities[i]);
                    }
                }

                outcomes.push_back({outcome, probability});
            }

            return outcomes;
        }

        static std::vector<std::pair<std::string, long long>> simulateOutcomes(const std::vector<double>& probabilities, long long n)
        {
            std::vector<std::pair<std::string, long long>> outcomes;
            int numEvents = probabilities.size();
            int totalOutcomes = 1 << numEvents;

            std::vector<long long> counts(totalOutcomes, 0);

            for(long long trial = 0; trial < n; trial++)
            {
                int mask = 0;
                for(int i = 0; i < numEvents; i++)
                {
                    if(task1::runSingle(probabilities[i]))
                    {
                        mask |= (1 << i);
                    }
                }
                counts[mask]++;
            }

            for(int mask = 0; mask < totalOutcomes; mask++)
            {
                std::string outcome;
                for(int i = 0; i < numEvents; i++)
                {
                    if(mask & (1 << i))
                        outcome += "T";
                    else
                        outcome += "F";
                }
                outcomes.push_back({outcome, counts[mask]});
            }

            return outcomes;
        }

        static std::string simulateSingleOutcome(const std::vector<double>& probabilities)
        {
            std::string result;
            for(const auto& p : probabilities)
            {
                result += task1::runSingle(p) ? "T" : "F";
            }
            return result;
        }

        static bool validateProbabilities(const std::vector<double>& probabilities)
        {
            if(probabilities.empty())
                return false;

            for(const auto& p : probabilities)
            {
                if(p < 0.0 || p > 1.0)
                    return false;
            }
            return true;
        }

        static void callback(Fl_Widget* button, void* data)
        {
            button->deactivate();

            task2DTO* tmp = static_cast<task2DTO*>(data);

            std::string input = tmp->in->value();
            long long n = tmp->n;

            Fl_Multiline_Output* out = tmp->out;
            Fl_Multiline_Output* out_theoretical = tmp->out_theoretical;

            std::thread calc(
                [input, n, button, out, out_theoretical]()
                {
                    std::vector<double> probabilities = processInput(input.c_str());
                    std::string* text_result = new std::string();
                    std::string* text_theoretical = new std::string();

                    if(!validateProbabilities(probabilities))
                    {
                        *text_result = "Error: Invalid input\nConstraints:\n- At least one probability required\n- Each probability must be in [0, 1]";
                        *text_theoretical = "";
                    }
                    else if(n == 1)
                    {
                        *text_result = simulateSingleOutcome(probabilities);
                        *text_theoretical = "";
                    }
                    else
                    {
                        if(n <= 0)
                        {
                            *text_result = "Error: N must be positive";
                            *text_theoretical = "";
                        }
                        else
                        {
                            auto theoreticalOutcomes = calculateOutcomes(probabilities);
                            auto simulatedOutcomes = simulateOutcomes(probabilities, n);

                            for(const auto& outcome : simulatedOutcomes)
                            {
                                double freq = (double)outcome.second / (double)n;
                                *text_result += outcome.first + ": " + std::to_string(freq) + "\n";
                            }
                            if(!text_result->empty() && text_result->back() == '\n')
                                text_result->pop_back();

                            for(const auto& outcome : theoreticalOutcomes)
                            {
                                *text_theoretical += outcome.first + ": " + std::to_string(outcome.second) + "\n";
                            }
                            if(!text_theoretical->empty() && text_theoretical->back() == '\n')
                                text_theoretical->pop_back();
                        }
                    }

                    Fl::awake(
                        [](void* userdata)
                        {
                            auto params =
                                static_cast<
                                    std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>*
                                    >(userdata);

                            Fl_Widget* btn = std::get<0>(*params);
                            Fl_Multiline_Output* out = std::get<1>(*params);
                            Fl_Multiline_Output* out_theo = std::get<2>(*params);
                            std::string* str = std::get<3>(*params);
                            std::string* str_theo = std::get<4>(*params);

                            out->value(str->c_str());
                            out_theo->value(str_theo->c_str());
                            btn->activate();
                            Fl::redraw();

                            delete str;
                            delete str_theo;
                            delete params;
                        },
                        new std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>(
                            button, out, out_theoretical, text_result, text_theoretical
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
            Fl_Multiline_Output* out_theoretical;
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
