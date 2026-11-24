/*
 * flash.h
 *
 *  Created on: 2025年6月13日
 *      Author: 吉祥止止
 */

#ifndef CODE_FLASH_H_
#define CODE_FLASH_H_


// Flash的存点相关参数

#define FLASH_SECTION_INDEX0       (0)
#define FLASH_GPS_INDEX0            (2)   //页码
#define FLASH_BUFFER_SIZE0         (512)

//科目一
#define FLASH_SECTION_INDEX       (0)
#define FLASH_GPS_INDEX           (6)   //页码
#define FLASH_BUFFER_SIZE         (512)

//科目二
#define FLASH_SECTION_INDEX1       (0)
#define FLASH_GPS_INDEX1           (7)  //页码
#define FLASH_BUFFER_SIZE1         (512)

//科目三
#define FLASH_SECTION_INDEX2       (0)
#define FLASH_GPS_INDEX2           (11) //页码
#define FLASH_BUFFER_SIZE2         (512)

#define Point_MAX   19

void FLASH_Get_KM1(void);
void FLASH_READ_KM1(void);

void FLASH_Get_KM2(void);
void FLASH_READ_KM2(void);

void FLASH_Get_KM3(void);
void FLASH_READ_KM3(void);

extern uint32 LAT_data1[Point_MAX];  //纬度存储数组
extern uint32 LOG_data1[Point_MAX];  //经度存储数组

#endif /* CODE_FLASH_H_ */
