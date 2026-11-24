/*
 * Karlm.h
 *
 *  Created on: 2025年7月2日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_KARLM_H_
#define CODE_KARLM_H_


typedef struct
{
    float Last_P;//上次估算协方差 不可以为0 ! ! ! ! !
    float Now_P;//当前估算协方差
    float out;//卡尔曼滤波器输出
    float Kg;//卡尔曼增益
    float Q;//过程噪声协方差
    float R;//观测噪声协方差
}Kalman;

void Kalman_Init(Kalman *kfp);
float KalmanFilter(Kalman *kfp,float input);

extern Kalman kfp_0;

typedef struct {
    float psi;      // 估计的偏航角（rad）
    float b;        // 估计的陀螺仪零漂（rad/s）

    float P[2][2];  // 协方差矩阵
    float Q[2];     // 过程噪声方差 [Q_psi, Q_b]
    float R;        // 测量噪声方差
    float dt;       // 采样周期（s）
} YawKF;

/**
 * @brief 初始化卡尔曼滤波器
 * @param kf       滤波器实例指针
 * @param dt       采样周期（秒）
 * @param Q_psi    偏航角过程噪声方差
 * @param Q_b      零漂过程噪声方差
 * @param R        测量噪声方差
 */
void YawKF_Init(YawKF *kf, float dt, float Q_psi, float Q_b, float R);

/**
 * @brief 执行一次卡尔曼滤波更新
 * @param kf       滤波器实例指针
 * @param gyro_z   陀螺仪 Z 轴角速度（rad/s）
 * @param yaw_meas 外部测量偏航角（rad, -π~π）
 */
void YawKF_Update(YawKF *kf, float gyro_z, float yaw_meas);



#endif /* CODE_KARLM_H_ */
