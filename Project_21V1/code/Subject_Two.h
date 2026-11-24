/*
 * Subject_Two.h
 *
 *  Created on: 2026年4月3日
 *      Author: liuliangfang
 */

#ifndef CODE_SUBJECT_TWO_H_
#define CODE_SUBJECT_TWO_H_

void Process_Command(uint8_t cmd);
void run_command_sequence(void);
void emergency_flasher(void);   //双闪灯
void left_turn_signal(void);    //左转
void dipped_headlight(void);    //近光灯
void headlights_on_full_beam(void); //远光灯
void Right_turn_signal(void);   //右转
void foglight(void);            //雾灯
void position_light(void);      //示廓灯
void windshield_wiper_on(void); //雨刮器

extern volatile uint8_t cmd_sequence_running;

#endif /* CODE_SUBJECT_TWO_H_ */
