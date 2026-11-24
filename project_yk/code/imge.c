/*
 * image.c
 *
 *  Created on: 2025年6月28日
 *      Author: liuliangfang
 */

#include "imge.h"
#include "zf_common_headfile.h"
#include <math.h>
//
//
//
//// 全局变量
//uint8_t base_image[IMG_HEIGHT][IMG_WIDTH];
//uint8_t left_line_list[IMG_HEIGHT];
//uint8_t right_line_list[IMG_HEIGHT];
//uint8_t mid_line_list[IMG_HEIGHT];
//float image_steering_duty;
//float mid_avg;
//PID_STEER_see pid_image;
//uint8_t temp_image[IMG_HEIGHT][IMG_WIDTH];  // 腐蚀处理临时图像
//
////#define SERVO_MOTOR_MID             (95)
//
//// 大津法阈值计算算法
//void mt9v03x_otsu_binarization(uint8_t (*image)[IMG_WIDTH], uint8_t (*binary_image)[IMG_WIDTH])
//{
//    uint32_t histogram[256] = {0};
//    const uint32_t total_pixels = IMG_HEIGHT * IMG_WIDTH;
//    uint32_t sum = 0, sumB = 0;
//    uint32_t q1 = 0;
//    uint16_t threshold = 0;
//    float max_var = 0.0f;
//
//    // 计算图像灰度直方图
//    uint8_t *img_ptr = (uint8_t *)image;
//    for (uint32_t i = 0; i < total_pixels; i++) {
//        histogram[img_ptr[i]]++;
//    }
//
//    // 计算总灰度值
//    for (uint16_t i = 0; i < 256; i++) {
//        sum += i * histogram[i];
//    }
//
//    // 遍历所有可能的阈值，找到最大类间方差对应的阈值
//    for (uint16_t t = 128; t < 256; t++) {  // 从128开始遍历阈值
//        q1 += histogram[t];
//        if (q1 == 0) continue;
//
//        uint32_t q2 = total_pixels - q1;
//        if (q2 == 0) break;
//
//        sumB += t * histogram[t];
//        float u1 = (float)sumB / q1;
//        float u2 = (float)(sum - sumB) / q2;
//        float var = q1 * q2 * (u1 - u2) * (u1 - u2);
//
//        if (var > max_var) {
//            max_var = var;
//            threshold = t;
//        }
//    }
//
//    // 应用阈值进行二值化
//    uint8_t *bin_ptr = (uint8_t *)binary_image;
//    for (uint32_t i = 0; i < total_pixels; i++) {
//        bin_ptr[i] = (img_ptr[i] >= threshold) ? WHITE : BLACK;
//    }
//}
//
//// 图像腐蚀处理 3x3模板
//void erode_image(uint8_t src[][IMG_WIDTH], uint8_t dst[][IMG_WIDTH], uint8_t times)
//{
//    // 0次腐蚀直接复制
//    if(times == 0) {
//        memcpy(dst, src, IMG_WIDTH * IMG_HEIGHT);
//        return;
//    }
//
//    // 第一次腐蚀
//    for (int i = 1; i < IMG_HEIGHT - 1; i++) {
//        for (int j = 1; j < IMG_WIDTH - 1; j++) {
//            if (src[i][j] == WHITE) {
//                // 检查3x3邻域是否全为白色
//                uint8_t all_white = 1;
//                for (int di = -1; di <= 1; di++) {
//                    for (int dj = -1; dj <= 1; dj++) {
//                        if (src[i+di][j+dj] != WHITE) {
//                            all_white = 0;
//                            break;
//                        }
//                    }
//                    if (!all_white) break;
//                }
//                temp_image[i][j] = all_white ? WHITE : BLACK;
//            } else {
//                temp_image[i][j] = BLACK;
//            }
//        }
//    }
//
//    // 后续腐蚀处理
//    for(int t = 1; t < times; t++) {
//        for (int i = 1; i < IMG_HEIGHT - 1; i++) {
//            for (int j = 1; j < IMG_WIDTH - 1; j++) {
//                if (temp_image[i][j] == WHITE) {
//                    // 检查3x3邻域是否全为白色
//                    uint8_t all_white = 1;
//                    for (int di = -1; di <= 1; di++) {
//                        for (int dj = -1; dj <= 1; dj++) {
//                            if (temp_image[i+di][j+dj] != WHITE) {
//                                all_white = 0;
//                                break;
//                            }
//                        }
//                        if (!all_white) break;
//                    }
//                    dst[i][j] = all_white ? WHITE : BLACK;
//                } else {
//                    dst[i][j] = BLACK;
//                }
//            }
//        }
//
//        // 更新临时图像用于下一次腐蚀
//        if(t < times - 1) {
//            memcpy(temp_image, dst, IMG_WIDTH * IMG_HEIGHT);
//        }
//    }
//
//    // 边界处理
//    for (int i = 0; i < IMG_HEIGHT; i++) {
//        dst[i][0] = BLACK;
//        dst[i][IMG_WIDTH-1] = BLACK;
//    }
//    for (int j = 0; j < IMG_WIDTH; j++) {
//        dst[0][j] = BLACK;
//        dst[IMG_HEIGHT-1][j] = BLACK;
//    }
//}
//
//
//
//// 定义 UNLABELED 和 MAX_LABELS
//#define UNLABELED 0
//#define MAX_LABELS 1000
//
//// 只保留最大的白色区域
//void keep_largest_white_region(uint8_t image[][IMG_WIDTH])
//{
//    uint16_t labels[IMG_HEIGHT][IMG_WIDTH] = {{0},{0}};
//    uint16_t current_label = 1;
//    uint16_t region_sizes[MAX_LABELS] = {0};
//    uint16_t equivalence[MAX_LABELS] = {0};
//
//    // 初始化等价数组
//    for (int i = 0; i < MAX_LABELS; i++) {
//        equivalence[i] = (uint16_t)i;
//    }
//
//    // 第一次遍历，标记连通区域
//    for (int i = 0; i < IMG_HEIGHT; i++) {
//        for (int j = 0; j < IMG_WIDTH; j++) {
//            if (image[i][j] == WHITE) {
//                uint16_t left_label = (j > 0) ? labels[i][j - 1] : UNLABELED;
//                uint16_t top_label = (i > 0) ? labels[i - 1][j] : UNLABELED;
//
//                if (left_label == UNLABELED && top_label == UNLABELED) {
//                    labels[i][j] = current_label++;
//                } else if (left_label != UNLABELED && top_label != UNLABELED) {
//                    // 合并标签
//                    uint16_t root_left = left_label;
//                    uint16_t root_top = top_label;
//                    while (equivalence[root_left] != root_left) {
//                        root_left = equivalence[root_left];
//                    }
//                    while (equivalence[root_top] != root_top) {
//                        root_top = equivalence[root_top];
//                    }
//                    if (root_left < root_top) {
//                        equivalence[root_top] = root_left;
//                    } else if (root_top < root_left) {
//                        equivalence[root_left] = root_top;
//                    }
//                    labels[i][j] = root_left;
//                } else if (left_label != UNLABELED) {
//                    labels[i][j] = left_label;
//                } else {
//                    labels[i][j] = top_label;
//                }
//            }
//        }
//    }
//
//    // 第二次遍历，将标签替换为根标签
//    for (int i = 0; i < IMG_HEIGHT; i++) {
//        for (int j = 0; j < IMG_WIDTH; j++) {
//            if (labels[i][j] != UNLABELED) {
//                uint16_t root = labels[i][j];
//                while (equivalence[root] != root) {
//                    root = equivalence[root];
//                }
//                labels[i][j] = root;
//                region_sizes[root]++;
//            }
//        }
//    }
//
//    // 找到最大区域的标签
//    uint16_t largest_label = 0;
//    uint16_t largest_size = 0;
//    for (int i = 1; i < current_label; i++) {
//        if (region_sizes[i] > largest_size) {
//            largest_size = region_sizes[i];
//            largest_label = (uint16_t)i;
//        }
//    }
//
//    // 只保留最大区域
//    for (int i = 0; i < IMG_HEIGHT; i++) {
//        for (int j = 0; j < IMG_WIDTH; j++) {
//            if (labels[i][j] != largest_label) {
//                image[i][j] = BLACK;
//            }
//        }
//    }
//}
//
//// 寻找中线算法
//void Find_Mid_Line(uint8_t image[][IMG_WIDTH])
//{
//    uint8_t left_point = 1;
//    uint8_t right_point = RIGHT_LIMIT;
//
//    // 初始化第一行边界
//    left_line_list[0] = 1;
//    right_line_list[0] = RIGHT_LIMIT;
//    mid_line_list[0] = (left_point + right_point) / 2;
//
//    // 扫描图像行 (从底部向上)
//    for (int i = IMG_HEIGHT - 1; i > 0; i--)
//    {
//        uint8_t found_left = 0;
//        uint8_t found_right = 0;
//
//        // 预定义扫描起始点
//        const uint8_t scan_points[] = {IMG_WIDTH/2, IMG_WIDTH/4, IMG_WIDTH*3/4};
//        const uint8_t num_points = sizeof(scan_points)/sizeof(scan_points[0]);
//
//        // 尝试不同起始点扫描
//        for (uint8_t idx = 0; idx < num_points; idx++)
//        {
//            uint8_t j = scan_points[idx];
//            if (image[i][j] != WHITE) continue;
//
//            // 向左扫描边界
//            for (int k = j; k > 1; k--) {
//                if (image[i][k] == BLACK && image[i][k+1] == WHITE) {
//                    left_point = (uint8_t)k;
//                    found_left = 1;
//                    break;
//                }
//            }
//
//            // 向右扫描边界
//            for (int k = j; k < RIGHT_LIMIT; k++) {
//                if (image[i][k] == BLACK && image[i][k-1] == WHITE) {
//                    right_point = (uint8_t)k;
//                    found_right = 1;
//                    break;
//                }
//            }
//
//            // 找到左右边界则跳出循环
//            if (found_left && found_right) break;
//        }
//
//        // 未找到左边界时全量扫描
//        if (!found_left) {
//            for (int j = 1; j < RIGHT_LIMIT; j++) {
//                if (image[i][j] == BLACK && image[i][j+1] == WHITE) {
//                    left_point = (uint8_t)j;
//                    break;
//                }
//            }
//        }
//
//        // 未找到右边界时全量扫描
//        if (!found_right) {
//            for (int j = RIGHT_LIMIT; j > 1; j--) {
//                if (image[i][j] == BLACK && image[i][j-1] == WHITE) {
//                    right_point = (uint8_t)j;
//                    break;
//                }
//            }
//        }
//
//        // 边界约束
//        left_point = CLAMP(left_point, 1, RIGHT_LIMIT);
//        right_point = CLAMP(right_point, 1, RIGHT_LIMIT);
//
//        // 记录边界和中线
//        left_line_list[i] = left_point;
//        right_line_list[i] = right_point;
//        mid_line_list[i] = (left_point + right_point) / 2;
//
//    }
//}
//
//// 绘制中线和边界
//void Find_Mid_Line_Draw_Line(void)
//{
//    for (uint8_t i = 0; i < IMG_HEIGHT - 2; i++) {
//        // 绘制左边界 (蓝色)
//        ips200_draw_point(left_line_list[i], i, RGB565_BLUE);
//        ips200_draw_point(left_line_list[i], i+1, RGB565_BLUE);
//        ips200_draw_point(left_line_list[i], i+2, RGB565_BLUE);
//
//        // 绘制右边界 (绿色)
//        ips200_draw_point(right_line_list[i], i, RGB565_GREEN);
//        ips200_draw_point(right_line_list[i], i+1, RGB565_GREEN);
//        ips200_draw_point(right_line_list[i], i+2, RGB565_GREEN);
//
//        // 绘制中线 (红色)
//        ips200_draw_point(mid_line_list[i], i, RGB565_RED);
//    }
//}
//
//float servo_motor_duty;
//
//// 图像转向计算
//void steering_image()
//{
//    // 有效帧判断：确保当前为有效帧
//    if(mt9v03x_finish_flag == 0) return;
//
//    // 加权中线和
//    float weighted_sum = 0;
//    float weight_total = 0;
//
//    for(int i = 0; i < SAMPLE_ROWS; i++)
//    {
//        // 动态权重计算（中间3行权重为3）
//        float weight = (i >= CORE_START && i <= CORE_END) ? 3.0f : 1.0f;
//
//        // 无效数据过滤
//        if(mid_line_list[i] < 1 || mid_line_list[i] > MAX_POS-1){
//            weight = 0; // 无效数据权重为0
//            continue;
//        }
//
//        weighted_sum += mid_line_list[i] * weight;
//        weight_total += weight;
//    }
//
//    // 异常处理：无有效数据时维持上一状态
//    if(weight_total < 0.1f) return;
//
//    // 计算加权平均值
//    mid_avg = weighted_sum / weight_total;
//
//    // 显示调试信息（可选）
//    ips200_show_float(0, 16*10, mid_avg, 3, 6);
//
//    // 转向控制范围映射（关键修改）
//    // 映射关系：0 -> 115, MAX_POS -> 75, MAX_POS/2 -> 95
//    float target_duty = 115.0f - (mid_avg / MAX_POS) * 40.0f;
//
//    // 确保目标占空比在有效范围
//    if(target_duty < 75.0f) target_duty = 75.0f;
//    if(target_duty > 115.0f) target_duty = 115.0f;
//
//    // 低通滤波平滑处理
//    static float filtered_duty = 95.0f; // 初始值为中心位置
//    filtered_duty = filtered_duty * (1-FILTER_FACTOR) + target_duty * FILTER_FACTOR;
//
//    // 确保滤波后的值在有效范围
//    if(filtered_duty < 75.0f) filtered_duty = 75.0f;
//    if(filtered_duty > 115.0f) filtered_duty = 115.0f;
//
//    image_steering_duty = filtered_duty;
//}
//
//double image_out;
//double expect_center = 87; ;
//float Target_view;
//uint8 view_flag = 0;
//
//// 新增：伽马变换函数实现
//void gamma_transform(uint8_t (*image)[IMG_WIDTH], float gamma)
//{
//    uint8_t gamma_lut[256];
//    for (int i = 0; i < 256; i++) {
//        float normalized = (float)i / 255.0f;
//        float gamma_corrected = powf(normalized, gamma);
//        gamma_lut[i] = (uint8_t)(gamma_corrected * 255.0f);
//    }
//
//    for (int i = 0; i < IMG_HEIGHT; i++) {
//        for (int j = 0; j < IMG_WIDTH; j++) {
//            image[i][j] = gamma_lut[image[i][j]];
//        }
//    }
//}
//
//// 图像控制函数
//void image_control(void)
//{
//    if (mt9v03x_finish_flag) {
//        // 伽马变换
//        float gamma = 1.5; // 可以根据需要调整伽马值
//        gamma_transform(mt9v03x_image, gamma);
//
//        // 图像二值化
//        mt9v03x_otsu_binarization(mt9v03x_image, base_image);  // 二值化
//
//        // 腐蚀处理 2 次
//        erode_image(base_image, base_image, ERODE_TIMES);
//
//        // 只保留最大的白色区域
//        keep_largest_white_region(base_image);
//
//        Find_Mid_Line(base_image);  // 寻找中线
//
//        // 显示原始图像
//        ips200_displayimage03x((const uint8 *)mt9v03x_image, IMG_WIDTH, IMG_HEIGHT);
//        Find_Mid_Line_Draw_Line();
//
//        // 转向计算
//        steering_image();
//
//        ips200_show_float(0, 16*12, image_steering_duty, 3, 6);
//
//        // 获取期望中心位置，图像中心
//       // expect_center = IMG_WIDTH / 2.0;
//        Target_view = expect_center - mid_avg;
//
//        // image_out = SERVO_MOTOR_MID + Loc_camera_PID(0, Target_view , &pid_image);
//
//        // 清除标志位
//        mt9v03x_finish_flag = 0;
//    }
//}
//double NUM;
//// 位置式PID计算函数
//double Loc_camera_PID(double expect_Angle, double Actual_Angle, PID_STEER_see *pid)
//{
//    pid->steer_error = expect_Angle - Actual_Angle;
//
//    // PID计算: P + D
//    pid->PID_result = pid->kp * pid->steer_error +
//                     pid->kd * (pid->steer_error - pid->steer_error_previous);
//
//    NUM = pid->kd * (pid->steer_error - pid->steer_error_previous);
//    // 保存上一次误差
//    pid->steer_error_previous = pid->steer_error;
//
//    // 限制输出范围 -20 到 20
//    pid->PID_result = CLAMP(pid->PID_result, -20, 20);
//
//    return pid->PID_result;
//}
//
//// PID控制参数初始化
//void Servo_camera_PID_Init(double kp, double kd)
//{
//    pid_image.kp = kp;
//    pid_image.kd = kd;
//    pid_image.steer_error_previous = 0;
//    pid_image.PID_result = 0;
//}
