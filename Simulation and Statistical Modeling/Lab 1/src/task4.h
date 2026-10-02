#pragma once

#include <random>
#include <vector>
#include <cmath>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <thread>
#include <string>

#include "config.h"
#include "utils.h"

class task4
{
private:
    inline static std::mt19937_64 gen{std::random_device{}()};
    inline static std::uniform_real_distribution<double> dist{0.0, 1.0};

public:
    static bool validateProbabilities(const std::vector<double>& probabilities, double* sum_out = nullptr)
    {
        if(probabilities.empty())
            return false;

        double sum = 0.0;
        for(const auto& p : probabilities)
        {
            if(p < 0.0 || p > 1.0)
                return false;
            sum += p;
        }

        if(sum_out)
            *sum_out = sum;

        if(std::abs(sum - 1.0) > 1e-9)
            return false;

        return true;
    }

    static int simulateSingle(const std::vector<double>& probabilities)
    {
        double rand_val = dist(gen);
        double cumulative = 0.0;

        for(size_t i = 0; i < probabilities.size(); i++)
        {
            cumulative += probabilities[i];
            if(rand_val < cumulative)
                return i;
        }

        return probabilities.size() - 1;
    }

    static std::vector<long long> simulateMultiple(const std::vector<double>& probabilities, long long n)
    {
        std::vector<long long> counts(probabilities.size(), 0);

        for(long long i = 0; i < n; i++)
        {
            int result = simulateSingle(probabilities);
            counts[result]++;
        }

        return counts;
    }

    static void callback_single(Fl_Widget* button, void* data)
    {
        task4DTO* dto = static_cast<task4DTO*>(data);

        std::string input = dto->in->value();
        std::vector<double> probabilities = processInput(input.c_str());

        if(!validateProbabilities(probabilities))
        {
            dto->output_experimental->value("Error: Invalid input\nConstraints:\n- At least one probability required\n- Each probability must be in [0, 1]\n- Sum of probabilities must equal 1.0");
            dto->output_theoretical->value("");
            return;
        }

        int result = simulateSingle(probabilities);
        std::string text = std::to_string(result);

        dto->output_experimental->value(text.c_str());

        std::string theoretical_text;
        for(size_t i = 0; i < probabilities.size(); i++)
        {
            theoretical_text += std::to_string(i) + ": " + std::to_string(probabilities[i]) + "\n";
        }
        dto->output_theoretical->value(theoretical_text.c_str());
    }

    static void callback_multiple(Fl_Widget* button, void* data)
    {
        button->deactivate();

        task4DTO* dto = static_cast<task4DTO*>(data);

        std::string input = dto->in->value();
        long long n;

        try {
            n = std::stoll(dto->n->value());
        } catch(...) {
            dto->output_experimental->value("Error: N must be a valid number");
            dto->output_theoretical->value("");
            button->activate();
            return;
        }

        if(n <= 0) {
            dto->output_experimental->value("Error: N must be positive");
            dto->output_theoretical->value("");
            button->activate();
            return;
        }

        Fl_Multiline_Output* output_exp = dto->output_experimental;
        Fl_Multiline_Output* output_theo = dto->output_theoretical;

        std::thread calc([button, output_exp, output_theo, input, n]()
        {
            std::vector<double> probabilities = processInput(input.c_str());

            if(!validateProbabilities(probabilities))
            {
                std::string* error_msg = new std::string("Error: Invalid input\nConstraints:\n- At least one probability required\n- Each probability must be in [0, 1]\n- Sum of probabilities must equal 1.0");
                std::string* empty_msg = new std::string("");
                Fl::awake([](void* userdata) {
                    auto params = static_cast<std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>*>(userdata);
                    std::get<1>(*params)->value(std::get<3>(*params)->c_str());
                    std::get<2>(*params)->value(std::get<4>(*params)->c_str());
                    std::get<0>(*params)->activate();
                    Fl::redraw();
                    delete std::get<3>(*params);
                    delete std::get<4>(*params);
                    delete params;
                }, new std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>(
                    button, output_exp, output_theo, error_msg, empty_msg));
                return;
            }

            std::vector<long long> counts = simulateMultiple(probabilities, n);

            std::string* experimental_text = new std::string();
            std::string* theoretical_text = new std::string();

            for(size_t i = 0; i < probabilities.size(); i++)
            {
                double experimental = (double)counts[i] / (double)n;
                *experimental_text += std::to_string(i) + ": " + std::to_string(experimental) + "\n";
                *theoretical_text += std::to_string(i) + ": " + std::to_string(probabilities[i]) + "\n";
            }

            Fl::awake(
                [](void* userdata)
                {
                    auto params = static_cast<std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>*>(userdata);

                    std::get<1>(*params)->value(std::get<3>(*params)->c_str());
                    std::get<2>(*params)->value(std::get<4>(*params)->c_str());
                    std::get<0>(*params)->activate();
                    Fl::redraw();

                    delete std::get<3>(*params);
                    delete std::get<4>(*params);
                    delete params;
                },
                new std::tuple<Fl_Widget*, Fl_Multiline_Output*, Fl_Multiline_Output*, std::string*, std::string*>(
                    button, output_exp, output_theo, experimental_text, theoretical_text
                )
            );
        });

        calc.detach();
    }

    struct task4DTO
    {
        Fl_Multiline_Input* in;
        Fl_Input* n;
        Fl_Multiline_Output* output_experimental;
        Fl_Multiline_Output* output_theoretical;
    };

private:
    static std::vector<double> processInput(const char* input)
    {
        std::vector<double> res;
        std::string text(input);
        std::string tmp;

        for(auto& i : text)
        {
            if(i == '\n')
            {
                try {
                    if(!tmp.empty())
                        res.push_back(std::stod(tmp));
                }
                catch(const std::exception& e) {}
                tmp = "";
                continue;
            }
            tmp += i;
        }

        if(!tmp.empty())
        {
            try {
                res.push_back(std::stod(tmp));
            }
            catch(const std::exception& e) {}
        }

        return res;
    }
};
