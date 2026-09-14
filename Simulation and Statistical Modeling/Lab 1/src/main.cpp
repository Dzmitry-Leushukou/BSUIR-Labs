#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_draw.H>


#include "config.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"

int main() {
    Fl::lock(); 
    FL_NORMAL_SIZE = 20;

    Fl_Window window(WW, WH, WTitle.c_str());
    window.color(grey_green);

    Fl_Input NInput(LNX, LNY, SNX, SNY, TN.c_str());
    NInput.type(FL_INT_INPUT); 
    NInput.value("1000000");

    Fl_Tabs tabs(TX, TY, WW-TX, WH-TY);
    tabs.color(dark_grey_green);
    tabs.begin();

        Fl_Group task1(TASK_X, TASK_Y, WW-TASK_X, WH-TASK_Y, "Task 1");
        task1.color(dark_grey_green);

        task1.begin();
            Fl_Box simulate_task1_border(SIMULATE_RECTANGLE_X, SIMULATE_RECTANGLE_Y,
            SIMULATE_RECTANGLE_SX, SIMULATE_RECTANGLE_SY);
            simulate_task1_border.box(FL_BORDER_BOX);
            simulate_task1_border.color(dark_grey_green);
            simulate_task1_border.labelcolor(FL_BLACK);

            Fl_Box simulate_task1_result(SIMULATE_RESULT_X, SIMULATE_RESULT_Y, 
                SIMULATE_RESULT_SX, SIMULATE_RESULT_SY, "RESULT");

            Fl_Input task1_p(PX, PY, PSX, PSY, PT.c_str());
            task1_p.type(FL_FLOAT_INPUT); 
            task1_p.value("0.5");

            Fl_Button simulate_task1_button(SIMULATE_TASK_BUTTON_X, SIMULATE_TASK_BUTTON_Y,
                SIMULATE_TASK_BUTTON_SX, SIMULATE_TASK_BUTTON_SY, SIMULATE_TASK_BUTTON_TEXT.c_str());
            task1::SingleDTO dto1{&task1_p, &simulate_task1_result};
            simulate_task1_button.callback(task1::simulate_callback, &dto1);
            
            Fl_Box simulate_mulptiple_task1_border(SIMULATE_MULTIPLE_RECTANGLE_X, SIMULATE_MULTIPLE_RECTANGLE_Y,
            SIMULATE_MULTIPLE_RECTANGLE_SX, SIMULATE_MULTIPLE_RECTANGLE_SY);
            simulate_mulptiple_task1_border.box(FL_BORDER_BOX);
            simulate_mulptiple_task1_border.color(dark_grey_green);
            simulate_mulptiple_task1_border.labelcolor(FL_BLACK);

            Fl_Box simulate_multiple_task1_result(SIMULATE_MULTIPLE_TASK_RESULT_X, SIMULATE_MULTIPLE_TASK_RESULT_Y, 
                SIMULATE_MULTIPLE_TASK_RESULT_SX, SIMULATE_MULTIPLE_TASK_RESULT_SY, SIMULATE_MULTIPLE_TASK_RESULT_TEXT.c_str());
            
            Fl_Button simulate_multiple_task1_button(SIMULATE_MULTIPLE_TASK_BUTTON_X,
            SIMULATE_MULTIPLE_TASK_BUTTON_Y, SIMULATE_MULTIPLE_TASK_BUTTON_SX,
            SIMULATE_MULTIPLE_TASK_BUTTON_SY, SIMULATE_MULTIPLE_TASK_BUTTON_TEXT.c_str());

            task1::MultipleDTO dto1m{&NInput, &task1_p, &simulate_multiple_task1_result};
            simulate_multiple_task1_button.callback(task1::simulate_multiple_callback, &dto1m);
        task1.end();
        
        Fl_Group task2(TASK_X,TASK_Y,WW-TASK_X,WH-TASK_Y,"Task 2");
        task2.color(dark_grey_green);
        task2.begin();
            Fl_Box probabilities_k_list_label(PROBABILITIES_K_LIST_LABEL_X, PROBABILITIES_K_LIST_LABEL_Y, 
                PROBABILITIES_K_LIST_LABEL_SX, PROBABILITIES_K_LIST_LABEL_SY, "K:");
            
            Fl_Multiline_Input input_task2 =  Fl_Multiline_Input(PROBABILITIES_LIST_INPUT_X, PROBABILITIES_LIST_INPUT_Y,
                 PROBABILITIES_LIST_INPUT_SX, PROBABILITIES_LIST_INPUT_SY);

            Fl_Multiline_Output output_task2 =  Fl_Multiline_Output(PROBABILITIES_LIST_OUTPUT_X,
                PROBABILITIES_LIST_OUTPUT_Y, PROBABILITIES_LIST_OUTPUT_SX, PROBABILITIES_LIST_OUTPUT_SY);

            
            
            Fl_Button simulate_single_task2_button(SIMULATE_K_LIST_PROBABILITIES_X,
            SIMULATE_K_LIST_PROBABILITIES_Y, SIMULATE_K_LIST_PROBABILITIES_SX,
            SIMULATE_K_LIST_PROBABILITIES_SY, SIMULATE_K_LIST_PROBABILITIES_TEXT.c_str());
            
            task2::task2DTO dto2 = task2::task2DTO{&input_task2, &output_task2, 1};
            simulate_single_task2_button.callback(task2::callback, &dto2);

            Fl_Box probabilities_result_k_list_label(PROBABILITIES_RESULT_K_LIST_LABEL_X,
                PROBABILITIES_RESULT_K_LIST_LABEL_Y, PROBABILITIES_RESULT_K_LIST_LABEL_SX,
                PROBABILITIES_RESULT_K_LIST_LABEL_SY, "RESULT:");
            
            
            Fl_Button simulate_multi_task2_button(SIMULATE_MULTI_LIST_PROBABILITIES_X,
            SIMULATE_MULTI_LIST_PROBABILITIES_Y, SIMULATE_MULTI_LIST_PROBABILITIES_SX,
            SIMULATE_MULTI_LIST_PROBABILITIES_SY, SIMULATE_MULTI_LIST_PROBABILITIES_TEXT.c_str());

            task2::task2DTO dto2n = task2::task2DTO{&input_task2, &output_task2, std::stoll(NInput.value())};
            simulate_multi_task2_button.callback(task2::callback, &dto2n);

        task2.end();

        Fl_Group task3(TASK_X,TASK_Y,WW-TASK_X,WH-TASK_Y,"Task 3");
        task3.color(dark_grey_green);
        task3.begin();

            Fl_Input task3_pa(PA_X, PA_Y, PA_SX, PA_SY, PA_TEXT.c_str());
            task3_pa.type(FL_FLOAT_INPUT); 
            task3_pa.value("0.5");

            Fl_Input task3_pbaa(PBaA_X, PBaA_Y, PBaA_SX, PBaA_SY, PBaA_TEXT.c_str());
            task3_pbaa.type(FL_FLOAT_INPUT); 
            task3_pbaa.value("0.5");

            Fl_Box task3_pab(PAB_X, PAB_Y, PAB_SX, PAB_SY, PAB_TEXT.c_str());
            Fl_Box task3_panb(PANB_X, PANB_Y, PANB_SX, PANB_SY, PANB_TEXT.c_str());
            Fl_Box task3_pnab(PNAB_X, PNAB_Y, PNAB_SX, PNAB_SY, PNAB_TEXT.c_str());
            Fl_Box task3_pnanb(PNANB_X, PNANB_Y, PNANB_SX, PNANB_SY, PNANB_TEXT.c_str());
            task3_pab.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_panb.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pnab.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pnanb.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

            Fl_Button task3_sbutton(SIMULATE_ST3_X, SIMULATE_ST3_Y, SIMULATE_ST3_SX, SIMULATE_ST3_SY,
            SIMULATE_ST3_TEXT.c_str());

            Fl_Button task3_mbutton(SIMULATE_MT3_X, SIMULATE_MT3_Y, SIMULATE_MT3_SX, SIMULATE_MT3_SY,
            SIMULATE_MT3_TEXT.c_str());

            
            Fl_Box task3_border(task3_border_x, task3_border_y,
            task3_border_sx, task3_border_sy);
            task3_border.box(FL_BORDER_BOX);
            task3_border.color(FL_BLACK);
            task3_border.labelcolor(FL_BLACK);


            Fl_Box t3hb1(t3hb1_x, t3hb1_y, t3hb1_sx, t3hb1_sy);
            t3hb1.box(FL_BORDER_BOX);
            t3hb1.color(FL_BLACK);
            t3hb1.labelcolor(FL_BLACK);

            Fl_Box t3hb2(t3hb2_x, t3hb2_y, t3hb2_sx, t3hb2_sy);
            t3hb2.box(FL_BORDER_BOX);
            t3hb2.color(FL_BLACK);
            t3hb2.labelcolor(FL_BLACK);

            Fl_Box t3hb3(t3hb3_x, t3hb3_y, t3hb3_sx, t3hb3_sy);
            t3hb3.box(FL_BORDER_BOX);
            t3hb3.color(FL_BLACK);
            t3hb3.labelcolor(FL_BLACK);

            Fl_Box t3hb4(t3hb4_x, t3hb4_y, t3hb4_sx, t3hb4_sy);
            t3hb4.box(FL_BORDER_BOX);
            t3hb4.color(FL_BLACK);
            t3hb4.labelcolor(FL_BLACK);
            
            Fl_Box t3hb5(t3hb5_x, t3hb5_y, t3hb5_sx, t3hb5_sy);
            t3hb5.box(FL_BORDER_BOX);
            t3hb5.color(FL_BLACK);
            t3hb5.labelcolor(FL_BLACK);

            Fl_Box t3hb6(t3hb6_x, t3hb6_y, t3hb6_sx, t3hb6_sy);
            t3hb6.box(FL_BORDER_BOX);
            t3hb6.color(FL_BLACK);
            t3hb6.labelcolor(FL_BLACK);

            Fl_Box task3_pa_res(task3_sc, PA_Y, PA_SX, PA_SY, "JOPA");
            Fl_Box task3_pbaa_res(task3_sc, PBaA_Y, PBaA_SX, PBaA_SY, "JOPA");
            Fl_Box task3_pab_res(task3_sc, PAB_Y, PAB_SX, PAB_SY, "JOPA");
            Fl_Box task3_panb_res(task3_sc, PANB_Y, PANB_SX, PANB_SY, "JOPA");
            Fl_Box task3_pnab_res(task3_sc, PNAB_Y, PNAB_SX, PNAB_SY, "JOPA");
            Fl_Box task3_pnanb_res(task3_sc, PNANB_Y, PNANB_SX, PNANB_SY, "JOPA");
            task3_pab_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_panb_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pnab_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pnanb_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pbaa_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            task3_pa_res.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

            task3::DTO t3dto = task3::DTO{&task3_pa_res, &task3_pbaa_res,
            &task3_pab, &task3_panb_res, &task3_pnab_res, &task3_pnanb_res,
            &task3_pab, &task3_panb, &task3_pnab, &task3_pnanb, &task3_pa,
            &task3_pbaa};


            task3_pa.when(FL_WHEN_CHANGED);
            task3_pa.callback(task3::input_changed, &t3dto);

        task3.end();

        Fl_Group task4(TASK_X,TASK_Y,WW-TASK_X,WH-TASK_Y,"Task 4");
        task4.color(dark_grey_green);
        task4.begin();

            Fl_Button check4(200,200,200,200,"4");

        task4.end();

    tabs.end();
    window.end(); 
    window.show();

    return Fl::run();
}
