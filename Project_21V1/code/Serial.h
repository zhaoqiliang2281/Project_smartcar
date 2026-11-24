/*
 * Serial.h
 *
 *  Created on: 2026年3月23日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_SERIAL_H_
#define CODE_SERIAL_H_

#include "zf_common_headfile.h"

#define Buffer1_Max 64

extern int32_t m;
extern int32_t x;
extern int32_t y;
extern int32_t z;

uint8_t Serial_Data_Resolving(int32_t *m, int32_t *x, int32_t *y, int32_t *z);

//调用示例
///*串口解析：接收数据为整形，串口助手以文本格式发送了，m%dx%dy%dz%d ；其中‘%d’为对应函数参数,能同时解析四个整数*/
//Serial_Data_Resolving(&m,&x,&y,&z);
//ips200_show_uint(    0 , 16*9,   m, 5);
//ips200_show_uint(    0 , 16*10,   (uint32)x, 5);
//ips200_show_uint(    0 , 16*11,   (uint32)y, 5);
//ips200_show_uint(    0 , 16*12,   (uint32)z, 5);

#endif /* CODE_SERIAL_H_ */
