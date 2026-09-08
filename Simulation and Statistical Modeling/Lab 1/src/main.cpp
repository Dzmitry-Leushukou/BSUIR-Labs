#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>

#include "config.h"
#include "task1.h"
#include "task2.h"


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
            
            Fl_Button simulate_single_task2_button(SIMULATE_K_LIST_PROBABILITIES_X,
            SIMULATE_K_LIST_PROBABILITIES_Y, SIMULATE_K_LIST_PROBABILITIES_SX,
            SIMULATE_K_LIST_PROBABILITIES_SY, SIMULATE_K_LIST_PROBABILITIES_TEXT.c_str());

            Fl_Box probabilities_result_k_list_label(PROBABILITIES_RESULT_K_LIST_LABEL_X,
                PROBABILITIES_RESULT_K_LIST_LABEL_Y, PROBABILITIES_RESULT_K_LIST_LABEL_SX,
                PROBABILITIES_RESULT_K_LIST_LABEL_SY, "RESULT:");
            
            Fl_Multiline_Output output_task2 =  Fl_Multiline_Output(PROBABILITIES_LIST_OUTPUT_X,
                PROBABILITIES_LIST_OUTPUT_Y, PROBABILITIES_LIST_OUTPUT_SX, PROBABILITIES_LIST_OUTPUT_SY);

            Fl_Button simulate_multi_task2_button(SIMULATE_MULTI_LIST_PROBABILITIES_X,
            SIMULATE_MULTI_LIST_PROBABILITIES_Y, SIMULATE_MULTI_LIST_PROBABILITIES_SX,
            SIMULATE_MULTI_LIST_PROBABILITIES_SY, SIMULATE_MULTI_LIST_PROBABILITIES_TEXT.c_str());
        task2.end();

        Fl_Group task3(TASK_X,TASK_Y,WW-TASK_X,WH-TASK_Y,"Task 3");
        task3.color(dark_grey_green);
        task3.begin();

            Fl_Button check3(200,200,200,200,"3");

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
