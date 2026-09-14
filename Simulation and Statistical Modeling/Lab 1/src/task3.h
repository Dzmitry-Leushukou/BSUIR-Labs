#pragma once

#include <FL/Fl_Box.H>
#include <FL/Fl_Widget.H>


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

        Fl_Box* pab;
        Fl_Box* panb;
        Fl_Box* pnab;
        Fl_Box* pnanb;

        Fl_Input* pa;
        Fl_Input* pbaa;

    };

    static void input_changed(Fl_Widget* button, void* data)
    {
        DTO* dto_tmp = static_cast<DTO*>(data);
        // P(A)
        double pa = dto_tmp->pa->value();
        // P(B|A)
        double pbaa = dto_tmp->pbaa->value();
        // P(!B|A)
        double pnbaa = 1.0 - pbaa;
        // P(!A)
        double pna = 1.0 - pa;
        // P(B|!A)
        double pnab = 1 - pbaa;

        // P(AB) = P(A) * P(B|A)
        double* pab = new double(pa * pbaa);
        // P(A!B)
        double* panb = new double(pnbaa * pa);
        // P(!AB)
        double * pnab = new double(pna * pa) ;
        // P(!A!B)
        double * pnanb = new double(pna * pa) ;
        
    }
    
};