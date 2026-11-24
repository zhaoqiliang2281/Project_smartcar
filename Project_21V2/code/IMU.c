/*
 * IMU.c
 *
 *  Created on: 2025��4��16��
 *      Author: LENOVO
 */
/*
 * IMU.c
 *
 *  Created on: 2026��6��13��
 *      Author: LiangZhaoqi
 *      �����Ż�,ʹ��IMU963�Ŀ������˲���������IMU6��������ǵĿ������˲�������ע��
 */
#include "zf_common_headfile.h"

#define DEG_TO_RAD   0.017453292519943f

double Roll,Pitch,Yaw;//�����ǽ����������
double yaw;

gyro_param_t Gyro_Offset;//��Ʈ����

#if IMU_660RA_Start
/*
 * ********************** ����IMU���� **********************
  __     ____   ___  _____   ___ __  __ _   _
 / /_   |  _ \ / _ \|  ___| |_ _|  \/  | | | |
| '_ \  | | | | | | | |_     | || |\/| | | | |
| (_) | | |_| | |_| |  _|    | || |  | | |_| |
 \___/  |____/ \___/|_|     |___|_|  |_|\___/

 * ********************** ����IMU���� **********************
 */
static const double dt = 0.001f;  // 1ms����
static uint8 initialized = 0;

static quaternion_t attitude = {
        .q0=1.0f,
        .q1=0.0f,
        .q2= 0.0f,
        .q3= 0.0f,
        .beta = 0.1f
};

/************************************************
 *  �������ƣ�IMU_quaternion_init
 *  ����˵������ʼ����Ԫ����̬���Ʋ���
 *  �����������
 *  ���������������̬��Ԫ���ͳ�ʼ����־
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע������ IMU_660RA ģʽ�µ���̬��ʼ��
 ************************************************/
static void IMU_quaternion_init(void)
{
    attitude.q0 = 1.0f;
    attitude.q1 = attitude.q2 = attitude.q3 = 0.0f;
    initialized = 1;
}

/************************************************
 *  �������ƣ�normalize_vector
 *  ����˵������������ٶ��������й�һ������
 *  ���������x/y/z ��������ָ��
 *  �����������һ�������������
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע���ڲ���������������Ԫ����̬����ʹ��
 ************************************************/
static void normalize_vector(double *x, double *y, double *z)
{
    double recipNorm = 1.0f / sqrtf(*x * *x + *y * *y + *z * *z);
    *x *= recipNorm;
    *y *= recipNorm;
    *z *= recipNorm;
}

static uint8 Angle_flag = 0;

/************************************************
 *  �������ƣ�get_euler_angles
 *  ����˵������ȡ��ǰ��̬��Ԫ����Ӧ��ŷ��������
 *  �����������
 *  ���������������̬�ṹ��ָ��
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע�����ϲ���ö�ȡ Roll/Pitch/Yaw
 ************************************************/
const quaternion_t* get_euler_angles(void)
{
    return &attitude;
}

/************************************************
 *  �������ƣ�quaternion_update
 *  ����˵�������ڼ��ٶȼƺ������Ǹ�����Ԫ����̬
 *  �����������
 *  �������������ȫ����̬������
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע��IMU_660RA ģʽ�µĺ�����̬���㺯��static
 ************************************************/
void imu660ra_quaternion_update(void)
{
    if(!initialized) return;

    imu660ra_get_acc();         // �������ٶȼƲɼ�
    imu660ra_get_gyro();        // �ɼ�����������

    // ��ȡ����������
    double gx = imu660ra_gyro_transition(imu660ra_gyro_x/10*10) * DEG_TO_RAD;
    double gy = imu660ra_gyro_transition(imu660ra_gyro_y/10*10) * DEG_TO_RAD;
    double gz = imu660ra_gyro_transition(imu660ra_gyro_z/10*10)* DEG_TO_RAD;

    double ax = imu660ra_acc_transition(imu660ra_acc_x/100*100);
    double ay = imu660ra_acc_transition(imu660ra_acc_y/100*100);
    double az = imu660ra_acc_transition(imu660ra_acc_z/100*100);
    normalize_vector(&ax, &ay, &az);
    // ��ǰ��̬Ԥ�����������
    double vx = 2.0f * (attitude.q1*attitude.q3 - attitude.q0*attitude.q2);
    double vy = 2.0f * (attitude.q0*attitude.q1 + attitude.q2*attitude.q3);
    double vz = attitude.q0*attitude.q0 - attitude.q1*attitude.q1
             - attitude.q2*attitude.q2 + attitude.q3*attitude.q3;

    // �������
    double ex = (ay*vz - az*vy);
    double ey = (az*vx - ax*vz);
    double ez = (ax*vy - ay*vx);

    // Ӧ�ò���
    gx += attitude.beta * ex;
    gy += attitude.beta * ey;
    gz += attitude.beta * ez;
    // ��Ԫ������
    double q0 = attitude.q0;
    double q1 = attitude.q1;
    double q2 = attitude.q2;
    double q3 = attitude.q3;

    attitude.q0 += (-q1*gx - q2*gy - q3*gz) * 0.5f * dt;
    attitude.q1 += ( q0*gx - q3*gy + q2*gz) * 0.5f * dt;
    attitude.q2 += ( q3*gx + q0*gy - q1*gz) * 0.5f * dt;
    attitude.q3 += (-q2*gx + q1*gy + q0*gz) * 0.5f * dt;

    // ��һ������
    double norm = sqrtf(attitude.q0*attitude.q0 + attitude.q1*attitude.q1 + attitude.q2*attitude.q2 + attitude.q3*attitude.q3);

    norm = 1.0f / norm;
    attitude.q0 *= norm;
    attitude.q1 *= norm;
    attitude.q2 *= norm;
    attitude.q3 *= norm;

    // ת��Ϊŷ���ǣ����ȣ�
    attitude.roll = atan2f(2*(q0*q1 + q2*q3), 1 - 2*(q1*q1 + q2*q2))*(1/DEG_TO_RAD);
    attitude.pitch = asinf(2*(q0*q2 - q3*q1))*(1/DEG_TO_RAD);
    attitude.yaw = -atan2f(2*(q0*q3 + q1*q2), 1 - 2*(q2*q2 + q3*q3))*(1/DEG_TO_RAD);

    const quaternion_t* angle = get_euler_angles();
    Roll  = angle->roll;
    Pitch = angle->pitch;
    Yaw   = angle->yaw;
    yaw   = Yaw;
    if(Yaw<0)  { Yaw+=360;}

    if(Angle_flag ==1)
    {
        IMU_quaternion_init();
        Angle_flag =0;
    }
}

void imu660ra_kalman_update(void)
{
    imu660ra_get_acc();
    imu660ra_get_gyro();

    Axis3f acc = {
        .x = imu660ra_acc_transition(imu660ra_acc_x),
        .y = imu660ra_acc_transition(imu660ra_acc_y),
        .z = imu660ra_acc_transition(imu660ra_acc_z)
    };

    Axis3f gyro = {
        .x = imu660ra_gyro_transition(imu660ra_gyro_x),
        .y = imu660ra_gyro_transition(imu660ra_gyro_y),
        .z = imu660ra_gyro_transition(imu660ra_gyro_z)
    };

    imu_update(acc, gyro, 0.001f);

    EulerAngles angles = imu_get_euler_angles(gyro);
    Roll  = angles.roll;
    Pitch = angles.pitch;
    Yaw   = angles.yaw;
    yaw   = Yaw;
    if(Yaw < 0.0f)
    {
        Yaw += 360.0f;
    }
}

#else
/*
 * ********************** ����IMU���� **********************
  ___    ____   ___  _____   ___ __  __ _   _
 / _ \  |  _ \ / _ \|  ___| |_ _|  \/  | | | |
| (_) | | | | | | | | |_     | || |\/| | | | |
 \__, | | |_| | |_| |  _|    | || |  | | |_| |
   /_/  |____/ \___/|_|     |___|_|  |_|\___/

 * ********************** ����IMU���� **********************
 */
IMU_param_t  IMU_Data;

/************************************************
 *  �������ƣ�IMU_gyro_Offset_Init
 *  ����˵�������������Ǿ�ֹʱ����Ưֵ�����ں���ȥƯ
 *  �����������
 *  ������������� Gyro_Offset ȫ����Ư����
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע����ʼ�������е��ã���֤���ٶȼ��㾫��
 ************************************************/
static void IMU_gyro_Offset_Init(void)
{
    Gyro_Offset.Xdata = 0;
    Gyro_Offset.Ydata = 0;
    Gyro_Offset.Zdata = 0;
    for (uint16_t i = 0; i < 1000; i++)
    {
        Gyro_Offset.Xdata += imu963ra_gyro_x;
        Gyro_Offset.Ydata += imu963ra_gyro_y;
        Gyro_Offset.Zdata += imu963ra_gyro_z;
        system_delay_ms(5);   // ��� 1Khz
    }

    Gyro_Offset.Xdata /= 1000;
    Gyro_Offset.Ydata /= 1000;
    Gyro_Offset.Zdata /= 1000;
}

/************************************************
 *  �������ƣ�imu_kalman_quaternion_update
 *  ����˵�������ÿ������ںϷ�ʽ������̬��
 *  �����������
 *  �������������ȫ�� Yaw �Ƕ�
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע��IMU_963RA ģʽ�µ���̬���º���
 ************************************************/
static void imu_kalman_quaternion_update(void)
{
    imu963ra_get_gyro();
    imu963ra_get_acc();
    Axis3f acc, gyro;
    // ���ٶ�ֵת��
    // �ڡ�8g�����£���ȡ���ļ��ٶȼ����ݳ��� 4098   ����ת��Ϊ��������λ������ ��λ g(m/s^2)
    acc.x = (float)imu963ra_acc_x / 4098.0f;
    acc.y = (float)imu963ra_acc_y / 4098.0f;
    acc.z = (float)imu963ra_acc_z / 4098.0f;

    // ������ֵת��
    // ����������Ϊ ��2000 dps ��ȡ�������������ݳ��� 14.3    ����ת��Ϊ��������λ������ ��λΪ ��/s
    gyro.x = (float)imu963ra_gyro_x / 14.3f;
    gyro.y = (float)imu963ra_gyro_y / 14.3f;
    gyro.z = (float)imu963ra_gyro_z / 14.3f;

    // ������̬���ƣ�dt=0.005�� (200Hz������)
    imu_update(acc, gyro, 0.001f);

    // ��ȡŷ���ǽ��
    EulerAngles angles = imu_get_euler_angles(gyro);

    // ����ȫ��ŷ���Ǳ����Թ�ʹ��
    //roll_Kalman = angles.roll;
    //pitch_Kalman = angles.pitch;
    Yaw = -angles.yaw;
    if(Yaw<0)//�滮Ϊ0-360��0-(-360)
    {
        Yaw+=360.0f;
    }

}

#endif

/************************************************
 *  �������ƣ�IMU_init
 *  ����˵������ʼ�� IMU �������������ƯУ׼׼��
 *  �����������
 *  �����������
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע������ IMU_660RA_Start ���л���ͬ IMU ����
 ************************************************/
void IMU_init(void)
{

#if IMU_660RA_Start

    imu660ra_init();            //�����ǳ�ʼ��
    IMU_quaternion_init();      //��Ԫ�ؽ����ʼ��
    imu_init();                 // ���� IMU963 ���˲�״̬��ʼ��

#else

    imu963ra_init();            //IMU963ra�ߵ���ʼ��
    IMU_gyro_Offset_Init();     // ��������Ư��ʼ��

#endif

}

/************************************************
 *  �������ƣ�Get_Angle
 *  ����˵������ȡ��ǰ��̬����Ϣ��Roll��Pitch��Yaw��
 *  �����������
 *  �������������ȫ����̬�Ǳ���
 *  �޸�ʱ�䣺2026��6��11��
 *  ��ע����ʱ�����е��ã������ⲿ��̬����
 ************************************************/
void Get_Angle(void)
{

#if IMU_660RA_Start

#if IMU_660RA_KALMAN

    imu660ra_kalman_update();

#else

    imu660ra_quaternion_update();

#endif

#else

    imu_kalman_quaternion_update();

#endif

}
//����
//printf("%f\n\r",Yaw);//��λ����ֵ
