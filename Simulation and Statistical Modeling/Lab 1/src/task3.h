#pragma once

#include <FL/Fl_Box.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input.H>
#include <string>
#include <cmath>
#include <random>


class task3
{
public:
    struct DTO
    {
        Fl_Box* pa_res;
        Fl_Box* pbaa_res;
        Fl_Box* pab_res;
        Fl_Box* panb_res;
        Fl_Box* pnab_res;
        Fl_Box* pnanb_res;

        Fl_Box* pab_theory;
        Fl_Box* panb_theory;
        Fl_Box* pnab_theory;
        Fl_Box* pnanb_theory;

        Fl_Input* pa;
        Fl_Input* pbaa;
        Fl_Input* n_input;
    };

    static int generate_compound_event(double pa, double pbaa, std::mt19937& gen, std::uniform_real_distribution<>& dis)
    {
        double r1 = dis(gen);
        bool event_a = (r1 < pa);

        double r2 = dis(gen);
        bool event_b;

        if(event_a) {
            event_b = (r2 < pbaa);
        } else {
            double pbana = 1.0 - pbaa;
            event_b = (r2 < pbana);
        }

        if(event_a && event_b) return 0;
        if(event_a && !event_b) return 1;
        if(!event_a && event_b) return 2;
        return 3;
    }

    static void input_changed(Fl_Widget* button, void* data)
    {
        DTO* dto_tmp = static_cast<DTO*>(data);

        double pa, pbaa;

        try {
            pa = std::stod(dto_tmp->pa->value());
            pbaa = std::stod(dto_tmp->pbaa->value());
        } catch(...) {
            dto_tmp->pab_theory->copy_label("Error: Invalid input");
            dto_tmp->panb_theory->copy_label("");
            dto_tmp->pnab_theory->copy_label("");
            dto_tmp->pnanb_theory->copy_label("");
            Fl::redraw();
            return;
        }

        if(pa < 0.0 || pa > 1.0 || pbaa < 0.0 || pbaa > 1.0) {
            dto_tmp->pab_theory->copy_label("Error: P(A) and P(B|A)");
            dto_tmp->panb_theory->copy_label("must be in [0, 1]");
            dto_tmp->pnab_theory->copy_label("");
            dto_tmp->pnanb_theory->copy_label("");
            Fl::redraw();
            return;
        }

        double pna = 1.0 - pa;
        double pnbaa = 1.0 - pbaa;
        double pbana = 1.0 - pbaa;
        double pnbana = pbaa;

        double pab = pa * pbaa;
        double panb = pa * pnbaa;
        double pnab = pna * pbana;
        double pnanb = pna * pnbana;

        std::string s_pab = "P(AB) = " + std::to_string(pab);
        std::string s_panb = "P(A!B) = " + std::to_string(panb);
        std::string s_pnab = "P(!AB) = " + std::to_string(pnab);
        std::string s_pnanb = "P(!A!B) = " + std::to_string(pnanb);

        dto_tmp->pab_theory->copy_label(s_pab.c_str());
        dto_tmp->panb_theory->copy_label(s_panb.c_str());
        dto_tmp->pnab_theory->copy_label(s_pnab.c_str());
        dto_tmp->pnanb_theory->copy_label(s_pnanb.c_str());

        dto_tmp->pab_theory->redraw();
        dto_tmp->panb_theory->redraw();
        dto_tmp->pnab_theory->redraw();
        dto_tmp->pnanb_theory->redraw();
    }

    static void simulate_multiple(Fl_Widget* button, void* data)
    {
        DTO* dto_tmp = static_cast<DTO*>(data);

        double pa, pbaa;
        long long n;

        try {
            pa = std::stod(dto_tmp->pa->value());
            pbaa = std::stod(dto_tmp->pbaa->value());
            n = std::stoll(dto_tmp->n_input->value());
        } catch(...) {
            dto_tmp->pab_res->copy_label("Error: Invalid input");
            dto_tmp->pab_res->redraw();
            return;
        }

        if(pa < 0.0 || pa > 1.0 || pbaa < 0.0 || pbaa > 1.0 || n <= 0) {
            dto_tmp->pab_res->copy_label("Error: Invalid parameters");
            dto_tmp->pab_res->redraw();
            return;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        long long count_ab = 0, count_anb = 0, count_nab = 0, count_nanb = 0;
        long long count_a = 0, count_b_given_a = 0;

        for(long long i = 0; i < n; i++) {
            int result = generate_compound_event(pa, pbaa, gen, dis);

            bool event_a = (result == 0 || result == 1);
            bool event_b = (result == 0 || result == 2);

            if(event_a) {
                count_a++;
                if(event_b) count_b_given_a++;
            }

            switch(result) {
                case 0: count_ab++; break;
                case 1: count_anb++; break;
                case 2: count_nab++; break;
                case 3: count_nanb++; break;
            }
        }

        double exp_pa = (double)count_a / n;
        double exp_pbaa = (count_a > 0) ? ((double)count_b_given_a / count_a) : 0.0;
        double exp_pab = (double)count_ab / n;
        double exp_panb = (double)count_anb / n;
        double exp_pnab = (double)count_nab / n;
        double exp_pnanb = (double)count_nanb / n;

        std::string s_exp_pa = std::to_string(exp_pa);
        std::string s_exp_pbaa = std::to_string(exp_pbaa);
        std::string s_exp_pab = std::to_string(exp_pab);
        std::string s_exp_panb = std::to_string(exp_panb);
        std::string s_exp_pnab = std::to_string(exp_pnab);
        std::string s_exp_pnanb = std::to_string(exp_pnanb);

        dto_tmp->pa_res->copy_label(s_exp_pa.c_str());
        dto_tmp->pbaa_res->copy_label(s_exp_pbaa.c_str());
        dto_tmp->pab_res->copy_label(s_exp_pab.c_str());
        dto_tmp->panb_res->copy_label(s_exp_panb.c_str());
        dto_tmp->pnab_res->copy_label(s_exp_pnab.c_str());
        dto_tmp->pnanb_res->copy_label(s_exp_pnanb.c_str());

        dto_tmp->pa_res->redraw();
        dto_tmp->pbaa_res->redraw();
        dto_tmp->pab_res->redraw();
        dto_tmp->panb_res->redraw();
        dto_tmp->pnab_res->redraw();
        dto_tmp->pnanb_res->redraw();
    }

};