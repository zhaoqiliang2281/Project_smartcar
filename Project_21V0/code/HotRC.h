/*
 * HotRC.h
 *
 *  Created on: 2025年6月15日
 *      Author: 吉祥止止
 */

#ifndef CODE_HOTRC_H_
#define CODE_HOTRC_H_

//extern uint8 Transform; //科目与手动开转换标志位
//extern uint8 Motor_Close_Temp;  //清除由踩油门由于一阶低通滤波的影响
//extern uint8 Motor_Start;   //电机开启标志位

void HotRC_control_Init(void);
void HotRC_receive_data_Show(void);
void HotRC_control(void);

//        /*遥控接收数据显示*/
//        HotRC_receive_data_Show();

//        /*遥控控制*/
//        HotRC_control();

#endif /* CODE_HOTRC_H_ */
