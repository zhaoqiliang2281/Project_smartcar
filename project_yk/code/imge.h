/*
 * image.h
 *
 *  Created on: 2025年6月28日
 *      Author: liuliangfang
 */

#ifndef CODE_IMGE_H_
#define CODE_IMGE_H_

//#include "zf_common_headfile.h"
//
//#define WHITE       255
//#define BLACK       0
//
//// 图像尺寸定义
//#define IMG_WIDTH       MT9V03X_W
//#define IMG_HEIGHT      MT9V03X_H
//#define RIGHT_LIMIT     (IMG_WIDTH - 1)  // 右边界索引
//#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
//
//// 腐蚀处理次数
//#define ERODE_TIMES      5     // 腐蚀次数
//
//// 中线计算参数
//#define SAMPLE_ROWS     20      // 采样行数
//#define CORE_START      5       // 核心区域起始行
//#define CORE_END        25      // 核心区域结束行
//#define MAX_POS         188.0f  // 最大位置值
//#define FILTER_FACTOR   0.3f    // 低通滤波系数
//
//// PID控制结构体
//typedef struct {
//    double kp;
//    double kd;
//    double steer_error;
//    double steer_error_previous;
//    double PID_result;
//} PID_STEER_see;
//extern double NUM;
//extern float mid_avg;
//extern PID_STEER_see pid_image;     // 摄像头PID结构体
//extern float image_steering_duty;
//extern float servo_motor_duty;
//extern double image_out;
//extern uint8_t temp_image[IMG_HEIGHT][IMG_WIDTH];  // 腐蚀处理临时图像
//
//extern double expect_center;
//extern float Target_view;
//extern uint8 view_flag;
//
//// 函数声明
//void mt9v03x_otsu_binarization(uint8_t (*image)[IMG_WIDTH], uint8_t (*binary_image)[IMG_WIDTH]);
//void erode_image(uint8_t src[][IMG_WIDTH], uint8_t dst[][IMG_WIDTH], uint8_t times);  // 图像腐蚀处理
//void Find_Mid_Line(uint8_t image[][IMG_WIDTH]);
//void Find_Mid_Line_Draw_Line(void);
//void steering_image(void);
//void image_control(void);
//void Servo_camera_PID_Init(double kp, double kd);
//double Loc_camera_PID(double expect_Angle, double Actual_Angle, PID_STEER_see *pid);
//void keep_largest_white_region(uint8_t image[][IMG_WIDTH]);
//// 新增：伽马变换函数声明
//void gamma_transform(uint8_t (*image)[IMG_WIDTH], float gamma);
//
#endif /* CODE_IMAGE_H_ */
