/*
 * mpu6050.c
 *
 *  Created on: May 16, 2025
 *      Author: diego
 */

#include "i2c.h"

float PI = 3.141592;
//float angulo_accel_pitch;//calcular pitch, se mueve el eje x, alrededor del y
//float angulo_accel_roll;//gira alrededor del eje x
//float angulo_filtro;

float offset_gx=0;
float offset_gy=0;
float offset_gz=0;
float offset_ax=0;
float offset_ay=0;
float offset_az=0;

float angulo_final;


void MPU6050_Init(void){

	//funciones para mandar y recibir con I2C
	//HAL_I2C_Mem_Read()-----param   I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout
	//HAL_I2C_Mem_Write()----param   I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout
	//
	//-------------------------------
	//setear el divisor del sample_rate del giroscopio para que ambos esten en 1000 muestras por segundo
	//------------------------------
	//la direccion se desplaza 1 a la izquierda ya que i2c utiliza 7 bits mas uno para escribir o leer, entonces le tienes que dar la direccion ya desplazada

	// Configurar DLPF primero
	uint8_t dlpf_config = 0x06; // Frecuencia de corte ~5Hz
	HAL_I2C_Mem_Write(&hi2c1, 0x68 << 1, 0x1A, 1, &dlpf_config, 1, HAL_MAX_DELAY);


	uint8_t sample_rate_div = 0x007;//0x007//15
	HAL_StatusTypeDef err = HAL_I2C_Mem_Write(&hi2c1,0x68 << 1,0x19,1,&sample_rate_div,1,100); // es mejor usar HAL_MAX_DELAY evita fallos y no agrega tiempos innecesarios

	/*
	 sample_rate_div = 0;//0x007//15
	 err = HAL_I2C_Mem_Read(&hi2c1,0x68 << 1,0x19,1,&sample_rate_div,1,100); // es mejor usar HAL_MAX_DELAY evita fallos y no agrega tiempos innecesarios
	__NOP();
	*/

	//-------------------------------
	//setear el full scale range +-250°/s del giroscopio, esto como prueba, en el balancin checar que opcion nos conviene
	//------------------------------
	//-------todo esto para no modificar los demas bits, solo los necesarios
	//--------------------------------

	// 1. PRIMERO leer el valor actual del registro
	uint8_t gyro_config;
	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x1B, 1, &gyro_config, 1, HAL_MAX_DELAY);


	// 2. Limpiar los bits 3-4 (FS_SEL) y establecerlos a 0b00
	gyro_config &= ~(0x3 << 3);  // Esto pone a 0 los bits 3 y 4
	// No necesitas OR porque 0b00 es el valor por defecto

	// 3. Escribir el nuevo valor
	HAL_I2C_Mem_Write(&hi2c1, 0x68 << 1, 0x1B, 1, &gyro_config, 1, HAL_MAX_DELAY);

	//--------------------------
	//setear el full scale range del acelerometro +-2g, cambiar luego solo de prueba
	//-----------------------
	// 1. PRIMERO leer el valor actual del registro
	uint8_t accel_config;
	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x1C, 1, &accel_config, 1, HAL_MAX_DELAY);


	// 2. Limpiar los bits 3-4 (FS_SEL) y establecerlos a 0b00
	accel_config &= ~(0x3 << 3);  // Esto pone a 0 los bits 3 y 4
	// No necesitas OR porque 0b00 es el valor por defecto

	// 3. Escribir el nuevo valor
	HAL_I2C_Mem_Write(&hi2c1, 0x68 << 1, 0x1C, 1, &accel_config, 1, HAL_MAX_DELAY);

	//------------------------
	//activar el mpu6050, quitar el sleep mode
	//------------------------
	uint8_t power_config;
	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x6B, 1, &power_config, 1, HAL_MAX_DELAY);
	power_config &=~(0b1<<6);
	HAL_I2C_Mem_Write(&hi2c1, 0x68 << 1, 0x6B, 1, &power_config, 1, HAL_MAX_DELAY);
	HAL_Delay(100);

	//---------------------------------
	//habilitando las interrupciones
	//------------------------------
	uint8_t pinInt= 0x00;
	HAL_I2C_Mem_Write(&hi2c1, 0x68<<1, 0x37, 1, &pinInt,1, HAL_MAX_DELAY);
	pinInt = 0x01;
	HAL_I2C_Mem_Write(&hi2c1, 0x68<<1, 0x38, 1, &pinInt,1,HAL_MAX_DELAY);

}



float MPU6050_Get_Ax(void){
	uint8_t buffer[2];
	int16_t raw_accel_x;
	float accel_x;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3B, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3B, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	raw_accel_x = (buffer[0]<<8) | buffer[1];

	//pasar a g's
	accel_x = raw_accel_x / 16384.0f;
	return ((accel_x*9.81)-offset_ax);
}

float MPU6050_Get_Ay(void){
	uint8_t buffer[2];
	int16_t raw_accel_y;
	float accel_y;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3D, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3D, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	raw_accel_y = (buffer[0]<<8) | buffer[1];

	//pasar a g's
	accel_y = raw_accel_y / 16384.0f;
	return ((accel_y*9.81)-offset_ay);
}

float MPU6050_Get_Az(void){
	uint8_t buffer[2];
	int16_t raw_accel_z;
	float accel_z;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3F, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x3F, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	raw_accel_z = (buffer[0]<<8) | buffer[1];

	//pasar a g's
	accel_z = raw_accel_z / 16384.0f;
	return ((accel_z*9.81)-offset_az);
}


float MPU6050_Get_Gx(void){
	uint8_t buffer[2];
	int16_t raw_gir_x;
	float gir_x;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x43, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x43, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	 raw_gir_x = (buffer[0]<<8) | buffer[1];

	//pasar a grados/s
	 gir_x = raw_gir_x / 131.0f;
	return gir_x-offset_gx;
}

float MPU6050_Get_Gy(void){
	uint8_t buffer[2];
	int16_t raw_gir_y;
	float gir_y;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x45, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x45, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	 raw_gir_y = (buffer[0]<<8) | buffer[1];

	//pasar a grados/s
	 gir_y = raw_gir_y / 131.0f;
	return gir_y-offset_gy;
}

float MPU6050_Get_Gz(void){
	uint8_t buffer[2];
	int16_t raw_gir_z;
	float gir_z;

	HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x47, 1, buffer, 2, HAL_MAX_DELAY);
	//aqui no es necesario poner que tmb lea 0x3C ya que al decirle que lea 2 bytes automaticamente se mueve a la siguiente direccion

	//verificar si se leyo bien el i2c
	 if(HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x47, 1, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
	        return 35.0f;  // O algún valor que indique error
	    }

	//poner los bits en orden, primero se lee el msb y luego el lsb
	 raw_gir_z = (buffer[0]<<8) | buffer[1];

	//pasar a grados/s
	 gir_z = raw_gir_z / 131.0f;
	return gir_z-offset_gz;
}

//funcion para calcular el angulo pero solo utilizando el acelerometro, este puede traer errores
float theta_accel_pitch(float ax,float ay, float az){
	float theta;
	//float ax;
	//float ay;
	//float az;
	//ax = MPU6050_Get_Ax();
	//ay = MPU6050_Get_Ay();
	//az = MPU6050_Get_Az();
	theta = atan2(-(ax),sqrt(ay*ay+az*az))*(180/PI);
	return theta;
}

float theta_accel_roll(float ax, float ay, float az){
	float theta;
	//float ax;
	//float ay;
	//float az;
	//ax = MPU6050_Get_Ax();
	//ay = MPU6050_Get_Ay();
	//az = MPU6050_Get_Az();
	theta = atan2((ay),sqrt(ax*ax+az*az))*(180/PI);
	return theta;
}

//implementacion del filtro complementario
//α: Factor de confianza (ej: 0.98 para giroscopio, 0.02 para acelerómetro).
//dt: Periodo de muestreo (ej: 0.01 segundos si mides a 100 Hz).

float theta_filtro(float pitch_angle_accel, float gy){
	float dt=0.001;//0.001 pq las mediciones del sensor son 1000 por segundo, ambos
	//float pitch_angle_accel;
	float alpha = 0.98;
	//float dt = 0.01;//muestreo ejemplo
	//pitch_angle_accel = theta_accel_pitch();
	//float gy;
	//gy = MPU6050_Get_Gy();

	//leer angulo con el giroscopio


	//aplicar filtro complementario
	angulo_final = alpha*(angulo_final+gy*dt)+(1-alpha)*pitch_angle_accel;


	return angulo_final;
}

void calibracion(void){
	float gx_sum=0;
	float gy_sum=0;
	float gz_sum=0;
	float ax_sum=0;
	float ay_sum=0;
	float az_sum=0;

	int n=100;
	for(int i=0;i<n;i++){
		gx_sum+= MPU6050_Get_Gx();
		gy_sum+= MPU6050_Get_Gy();
		gz_sum+= MPU6050_Get_Gz();
		ax_sum+= MPU6050_Get_Ax();
		ay_sum+= MPU6050_Get_Ay();
		az_sum+= MPU6050_Get_Az();
		HAL_Delay(20);
	}
	//acelerometro
	offset_ax = ax_sum/n;
	offset_ay = ay_sum/n;
	offset_az = (az_sum/n)-9.81f;
	//giroscopio
	offset_gx = gx_sum/n;
	offset_gy = gy_sum/n;
	offset_gz = gz_sum/n;
}
