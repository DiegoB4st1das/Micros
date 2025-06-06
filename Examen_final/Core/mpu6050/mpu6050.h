/*
 * mpu6050.h
 *
 *  Created on: May 16, 2025
 *      Author: diego
 */

#ifndef MPU6050_H_
#define MPU6050_H_



#endif /* MPU6050_H_ */


void MPU6050_Init(void);
float MPU6050_Get_Ax(void);
float MPU6050_Get_Ay(void);
float MPU6050_Get_Az(void);
float MPU6050_Get_Gx(void);
float MPU6050_Get_Gy(void);
float MPU6050_Get_Gz(void);
float theta_accel_pitch(float ax,float ay, float az);
float theta_accel_roll(float ax, float ay, float az);
float theta_filtro(float pitch_angle_accel, float gy);
void calibracion(void);

