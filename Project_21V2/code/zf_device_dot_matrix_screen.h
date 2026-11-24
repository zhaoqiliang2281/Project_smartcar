
#ifndef _zf_device_dot_matrix_screen_h_
#define _zf_device_dot_matrix_screen_h_


#include "zf_common_typedef.h"


#define DOT_MATRIX_SCREEN_SYNC_PIN      (ERU_CH7_REQ11_P20_9)

#define DOT_MATRIX_SCREEN_SR0_PIN       (P20_8)         //P23_1
#define DOT_MATRIX_SCREEN_SR1_PIN       (P33_10)        //P33_13
#define DOT_MATRIX_SCREEN_SR2_PIN       (P21_5)         //P15_8
#define DOT_MATRIX_SCREEN_SR3_PIN       (P21_7)         //P33_8
#define DOT_MATRIX_SCREEN_SR4_PIN       (P14_4)
#define DOT_MATRIX_SCREEN_SR5_PIN       (P14_5)
#define DOT_MATRIX_SCREEN_SR6_PIN       (P14_6)

#define DOT_MATRIX_SCREEN_ROW_NUM       (7)

void dot_matrix_screen_scan             (void);
void dot_matrix_screen_show_string      (const char *str);
void dot_matrix_screen_set_brightness   (uint16 brightness);
void dot_matrix_screen_init             (void);


void dot_matrix_screen_scan1(void);

extern int8    dot_matrix_screen_data[3];

#endif

