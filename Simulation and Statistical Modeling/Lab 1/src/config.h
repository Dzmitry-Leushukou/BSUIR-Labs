#pragma once

#include <FL/Enumerations.H>  
#include <FL/fl_draw.H>
#include <string>



//Window
const Fl_Color grey_green = fl_rgb_color(157, 201, 157);
const int WW = 600;
const int WH = 370;
const std::string WTitle = "Lab 1";

//"N: " label
const int SNY = 23;
const int SNX = WW - 40; //200 - RIGHT CORNER
const int LNY = 3;
const int LNX = WW - SNX - 2;
const std::string TN = "N: ";

//Tabs container
const int TX = 0;
const int TY = 30;
const Fl_Color dark_grey_green = fl_rgb_color(167, 191, 167);

//Task Size
const int TASK_X = TX;
const int TASK_Y = TY + 30;

//Task 1 preset
const int SIMULATE_TASK_BUTTON_X = TASK_X + 15;
const int SIMULATE_TASK_BUTTON_Y = TASK_Y + 60;
const int SIMULATE_TASK_BUTTON_SX = 200;
const int SIMULATE_TASK_BUTTON_SY = 50;
const std::string SIMULATE_TASK_BUTTON_TEXT = "SIMULATE";

const int SIMULATE_RESULT_X = SIMULATE_TASK_BUTTON_X;
const int SIMULATE_RESULT_Y = SIMULATE_TASK_BUTTON_Y - 40;
const int SIMULATE_RESULT_SX = SIMULATE_TASK_BUTTON_SX;
const int SIMULATE_RESULT_SY = SIMULATE_TASK_BUTTON_SY;

const int SIMULATE_RECTANGLE_X = SIMULATE_RESULT_X - 5;
const int SIMULATE_RECTANGLE_Y = SIMULATE_RESULT_Y - 5;
const int SIMULATE_RECTANGLE_SX = SIMULATE_TASK_BUTTON_SX + 10;
const int SIMULATE_RECTANGLE_SY = SIMULATE_TASK_BUTTON_SY + SIMULATE_RESULT_SY + 10;

const int PY = SIMULATE_RECTANGLE_Y;
const int PX = SIMULATE_RECTANGLE_X + SIMULATE_RECTANGLE_SX + 50; //200 - RIGHT CORNER
const int PSY = 40;
const int PSX = 200;
const std::string PT = "P: ";

const int SIMULATE_MULTIPLE_TASK_BUTTON_X = SIMULATE_TASK_BUTTON_X;
const int SIMULATE_MULTIPLE_TASK_BUTTON_Y = SIMULATE_TASK_BUTTON_Y + 150;
const int SIMULATE_MULTIPLE_TASK_BUTTON_SX = WW - 30;
const int SIMULATE_MULTIPLE_TASK_BUTTON_SY = 50;
const std::string SIMULATE_MULTIPLE_TASK_BUTTON_TEXT = "SIMULATE N EVENTS";

const int SIMULATE_MULTIPLE_TASK_RESULT_X = SIMULATE_MULTIPLE_TASK_BUTTON_X;
const int SIMULATE_MULTIPLE_TASK_RESULT_Y = SIMULATE_MULTIPLE_TASK_BUTTON_Y - 40;
const int SIMULATE_MULTIPLE_TASK_RESULT_SX = SIMULATE_MULTIPLE_TASK_BUTTON_SX;
const int SIMULATE_MULTIPLE_TASK_RESULT_SY = SIMULATE_MULTIPLE_TASK_BUTTON_SY;
const std::string SIMULATE_MULTIPLE_TASK_RESULT_TEXT = "Multiple run result: ";

const int SIMULATE_MULTIPLE_RECTANGLE_X = SIMULATE_MULTIPLE_TASK_RESULT_X - 5;
const int SIMULATE_MULTIPLE_RECTANGLE_Y = SIMULATE_MULTIPLE_TASK_RESULT_Y - 5;
const int SIMULATE_MULTIPLE_RECTANGLE_SX = SIMULATE_MULTIPLE_TASK_BUTTON_SX + 10;
const int SIMULATE_MULTIPLE_RECTANGLE_SY = SIMULATE_MULTIPLE_TASK_BUTTON_SY + SIMULATE_MULTIPLE_TASK_RESULT_SY + 10;

//Task 2 preset
const int PROBABILITIES_K_LIST_LABEL_X = 5;
const int PROBABILITIES_K_LIST_LABEL_Y = TASK_Y + 10;
const int PROBABILITIES_K_LIST_LABEL_SX = 20;
const int PROBABILITIES_K_LIST_LABEL_SY = 15;

const int PROBABILITIES_LIST_INPUT_X = PROBABILITIES_K_LIST_LABEL_X;
const int PROBABILITIES_LIST_INPUT_Y = PROBABILITIES_K_LIST_LABEL_Y + PROBABILITIES_K_LIST_LABEL_SY + 10;
const int PROBABILITIES_LIST_INPUT_SX = WW / 3;
const int PROBABILITIES_LIST_INPUT_SY = WH - PROBABILITIES_LIST_INPUT_Y - 50;

const int SIMULATE_K_LIST_PROBABILITIES_X = PROBABILITIES_LIST_INPUT_X;
const int SIMULATE_K_LIST_PROBABILITIES_Y = PROBABILITIES_LIST_INPUT_Y + PROBABILITIES_LIST_INPUT_SY + 5;
const int SIMULATE_K_LIST_PROBABILITIES_SX = PROBABILITIES_LIST_INPUT_SX;
const int SIMULATE_K_LIST_PROBABILITIES_SY = 40;
const std::string SIMULATE_K_LIST_PROBABILITIES_TEXT = "SIMULATE";

const int PROBABILITIES_RESULT_K_LIST_LABEL_X = PROBABILITIES_LIST_INPUT_X + PROBABILITIES_LIST_INPUT_SX + 50;
const int PROBABILITIES_RESULT_K_LIST_LABEL_Y = PROBABILITIES_K_LIST_LABEL_Y;
const int PROBABILITIES_RESULT_K_LIST_LABEL_SX = PROBABILITIES_LIST_INPUT_SX;
const int PROBABILITIES_RESULT_K_LIST_LABEL_SY = PROBABILITIES_K_LIST_LABEL_SY;

const int PROBABILITIES_LIST_OUTPUT_X = PROBABILITIES_RESULT_K_LIST_LABEL_X;
const int PROBABILITIES_LIST_OUTPUT_Y = PROBABILITIES_LIST_INPUT_Y;
const int PROBABILITIES_LIST_OUTPUT_SX = PROBABILITIES_LIST_INPUT_SX;
const int PROBABILITIES_LIST_OUTPUT_SY = PROBABILITIES_LIST_INPUT_SY;

const int SIMULATE_MULTI_LIST_PROBABILITIES_X = SIMULATE_K_LIST_PROBABILITIES_X + SIMULATE_K_LIST_PROBABILITIES_SX + 40;
const int SIMULATE_MULTI_LIST_PROBABILITIES_Y = SIMULATE_K_LIST_PROBABILITIES_Y;
const int SIMULATE_MULTI_LIST_PROBABILITIES_SX = SIMULATE_K_LIST_PROBABILITIES_SX + 20;
const int SIMULATE_MULTI_LIST_PROBABILITIES_SY = SIMULATE_K_LIST_PROBABILITIES_SY;
const std::string SIMULATE_MULTI_LIST_PROBABILITIES_TEXT = "SIMULATE N ACTIONS";

// Task 3 preset
const int PA_X = 80;
const int PA_Y = TASK_Y + 10;
const int PA_SX = 200;
const int PA_SY = 30;
const std::string PA_TEXT = "P(A):";

const int PBaA_X = PA_X;
const int PBaA_Y = PA_Y + PA_SY + 5;
const int PBaA_SX = PA_SX;
const int PBaA_SY = PA_SY;
const std::string PBaA_TEXT = "P(B | A):";


// Auto set fields
const int PAB_X = 0;
const int PAB_Y = PBaA_Y + PBaA_SY + 5;
const int PAB_SX = PA_SX + 80;
const int PAB_SY = PA_SY;
const std::string PAB_TEXT = "  P(AB) = ";

const int PANB_X = PAB_X;
const int PANB_Y = PAB_Y + PAB_SY + 5;
const int PANB_SX = PAB_SX;
const int PANB_SY = PAB_SY;
const std::string PANB_TEXT = " P(A!B) = ";

const int PNAB_X = PANB_X;
const int PNAB_Y = PANB_Y + PANB_SY + 5;
const int PNAB_SX = PANB_SX;
const int PNAB_SY = PANB_SY;
const std::string PNAB_TEXT = " P(!AB) = ";

const int PNANB_X = PNAB_X;
const int PNANB_Y = PNAB_Y + PNAB_SY + 5;
const int PNANB_SX = PNAB_SX;
const int PNANB_SY = PNAB_SY;
const std::string PNANB_TEXT = "P(!A!B) = ";


const int SIMULATE_ST3_X = 0;
const int SIMULATE_ST3_Y = PNANB_Y + PNANB_SY + 15;
const int SIMULATE_ST3_SX = PNANB_SX;
const int SIMULATE_ST3_SY = PNANB_SY+3;
const std::string SIMULATE_ST3_TEXT = "SIMULATE";

const int SIMULATE_MT3_X = SIMULATE_ST3_X;
const int SIMULATE_MT3_Y = SIMULATE_ST3_Y + SIMULATE_ST3_SY + 7;
const int SIMULATE_MT3_SX = SIMULATE_ST3_SX;
const int SIMULATE_MT3_SY = SIMULATE_ST3_SY;
const std::string SIMULATE_MT3_TEXT = "SIMULATE N ACTIONS";


const int task3_border_x = SIMULATE_ST3_X + SIMULATE_MT3_SX + 5;
const int task3_border_y = 0;
const int task3_border_sx = 2;
const int task3_border_sy = WH;

const int t3hb1_x = 0;
const int t3hb1_y = (PA_Y + PA_SY + PBaA_Y)/2;
const int t3hb1_sx = WW;
const int t3hb1_sy = 1;

const int t3hb2_x = 0;
const int t3hb2_y = (PAB_Y + PBaA_SY + PBaA_Y)/2;
const int t3hb2_sx = WW;
const int t3hb2_sy = 1;

const int t3hb3_x = 0;
const int t3hb3_y = (PAB_Y + PAB_SY + PANB_Y)/2;
const int t3hb3_sx = WW;
const int t3hb3_sy = 1;

const int t3hb4_x = 0;
const int t3hb4_y = (PANB_Y + PANB_SY + PNAB_Y)/2;
const int t3hb4_sx = WW;
const int t3hb4_sy = 1;

const int t3hb5_x = 0;
const int t3hb5_y = (PNAB_Y + PNAB_SY + PNANB_Y)/2;
const int t3hb5_sx = WW;
const int t3hb5_sy = 1;

const int t3hb6_x = 0;
const int t3hb6_y = PNANB_Y + PNANB_SY + 3;
const int t3hb6_sx = WW;
const int t3hb6_sy = 1;

const int task3_sc = task3_border_x + 1;
