/*
 * Karlm.c
 *
 *  Created on: 2025年7月2日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

 Kalman kfp;
 Kalman kfp_0;
void Kalman_Init(Kalman *kfp)
{
    kfp->Last_P = 1;
    kfp->Now_P = 0;
    kfp->out = 0;
    kfp->Kg = 0;
    kfp->Q = 0.5;
    kfp->R = 0.05;
}

/**
 *卡尔曼滤波器
 *@param    Kalman *kfp 卡尔曼结构体参数
 *              float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float KalmanFilter(Kalman *kfp,float input)
{
   //预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
   kfp->Now_P = kfp->Last_P + kfp->Q;
   //卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
   kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
   //更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
   kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//因为这一次的预测值就是上一次的输出值
   //更新协方差方程: 本次的系统协方差付给 kfp->LastP 威下一次运算准备。
   kfp->Last_P = (1-kfp->Kg) * kfp->Now_P;
   return kfp->out;
}

/*=======================================================

4. 关键调参与优化
若静止时仍有缓慢漂移：
① 检查磁力计校准（做“8字校准”）；
② 减小 R（如 0.05）或增大 Q_b（如 5e-6）。
若运动时角度抖动大：
① 增大 R（如 0.2）；
② 检查陀螺仪量程（±2000 °/s 时灵敏度为 131 LSB/(°/s)）。
若响应滞后：
① 增大 Q_psi（如 2e-4）；
② 提高采样频率（缩短 dt）。

=========================================================*/
void YawKF_Init(YawKF *kf, float dt, float Q_psi, float Q_b, float R) {
    kf->psi = 0.0f;
    kf->b   = 0.0f;

    kf->P[0][0] = 1.0f;  kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;  kf->P[1][1] = 1.0f;

    kf->Q[0] = Q_psi;
    kf->Q[1] = Q_b;
    kf->R    = R;
    kf->dt   = dt;
}

void YawKF_Update(YawKF *kf, float gyro_z, float yaw_meas) {
    // 1. 预测
    float psi_pred = kf->psi + (gyro_z - kf->b) * kf->dt;
    float b_pred   = kf->b;

    // 2. 预测协方差
    float P00 = kf->P[0][0] + kf->Q[0] + kf->dt*kf->dt*kf->P[1][1];
    float P01 = kf->P[0][1] - kf->dt*kf->P[1][1];
    float P10 = kf->P[1][0] - kf->dt*kf->P[1][1];
    float P11 = kf->P[1][1] + kf->Q[1];

    // 3. 更新
    float y = yaw_meas - psi_pred;          // 测量残差
    float S = P00 + kf->R;                  // 残差协方差
    float K0 = P00 / S;
    float K1 = P10 / S;

    kf->psi = psi_pred + K0 * y;
    kf->b   = b_pred   + K1 * y;

    // 4. 协方差更新
    kf->P[0][0] = (1.0f - K0) * P00;
    kf->P[0][1] = (1.0f - K0) * P01;
    kf->P[1][0] = P10 - K1 * P00;
    kf->P[1][1] = P11 - K1 * P10;
}

