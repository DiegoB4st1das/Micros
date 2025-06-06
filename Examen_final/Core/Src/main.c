/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "mpu6050.h"

#define PID_SAMPLING_RATE	(1000)
#define PID_MAX_OUTPUT 999               // Valor máximo de salida del PID
#define PID_MIN_OUTPUT -999              // Valor mínimo de salida del PID


float aceleracion_x;
float aceleracion_y;
float aceleracion_z;
float giroscopio_x;
float giroscopio_y;
float giroscopio_z;

float angulo_accel_pitch;//calcular pitch, se mueve el eje x, alrededor del y
float angulo_accel_roll;//gira alrededor del eje x
float angulo_filtro;


//variables para calibracion de mpu6050
/*

*/
//variables para hacer las mediciones sincronas

//float angulo_final;

//variables pid
float kp=150.0f; //puse 1 de ejemplo
float ki=0.0f;
float kd=0.0f;
float dt=0.001f;//muestreo cada 1 mili
float error_anterior = 0;
float output_tmp=0;

float setpoint = 0.4;
float output = 0;
//variable interrupcion
int ucFlag=0;



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

//funcion para lo del pwm
void HAL_PWM_SetDuty(uint32_t dc)
{
	if(dc > TIM3->ARR) TIM3->CCR1 = TIM3->ARR;
	else TIM3->CCR1 = dc;
}

//funcion pid
void funcionPID(float angulo_actual, float setpoint, float kp, float ki, float kd){
	// Variables estáticas para mantener los valores entre llamadas
	static float integral = 0;
	static float error_anterior = 0;
    static float derivada = 0;


	// Calculamos el error (diferencia entre lo que quieres y lo que tienes)
	float error = setpoint - angulo_actual;

	// aplicamos formula y multiplicamos el error*kp
    float proporcional = kp * error;

	// Calcular derivada
	derivada = (error-error_anterior)/dt;
	float termino_derivativo = kd * derivada;

    // Implementación de anti-windup para término integral
    if (integral > PID_MAX_OUTPUT) {
        integral = PID_MAX_OUTPUT;
        // Solo acumular si ayuda a reducir el error
        if (error < 0) integral += error * dt;
    } else if (integral < PID_MIN_OUTPUT) {
        integral = PID_MIN_OUTPUT;
        // Solo acumular si ayuda a reducir el error
        if (error > 0) integral += error * dt;
    } else {
        // Acumulación normal del término integral
        integral += error * dt;
    }

    float termino_integral = ki * integral;

    output = proporcional + termino_integral + termino_derivativo;


    if (output > PID_MAX_OUTPUT) {
            output = PID_MAX_OUTPUT;
    } else if (output < PID_MIN_OUTPUT){
            output = PID_MIN_OUTPUT;
    }
    output_tmp = output;
    // Control de dirección del motor basado en signo de u
    //gpioA3=in1
    //gpioA4=in2
    //gpioA5=in3
    //gpioB4=in4
    if (output < 0.0F) { // Rotación en sentido horario
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        output = -output; // Valor absoluto para PWM
    }
    else if(output >= 0.0F) { // Rotación en sentido antihorario
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    }

	error_anterior=error;

	HAL_PWM_SetDuty((uint32_t)output);

}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  MPU6050_Init();
  calibracion();
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  //checar cada cuanto se debe de aplicar el pid, que es cada que esten listo los datos
		if(ucFlag){
			ucFlag=0;
			aceleracion_x = MPU6050_Get_Ax();
			aceleracion_y = MPU6050_Get_Ay();
			aceleracion_z = MPU6050_Get_Az();
			giroscopio_x = MPU6050_Get_Gx();
			giroscopio_y = MPU6050_Get_Gy();
			giroscopio_z = MPU6050_Get_Gz();
			angulo_accel_pitch = theta_accel_pitch(aceleracion_x, aceleracion_y, aceleracion_z);
			angulo_accel_roll = theta_accel_roll(aceleracion_x, aceleracion_y, aceleracion_z);
			angulo_filtro = theta_filtro(angulo_accel_pitch,giroscopio_y);
			funcionPID(angulo_filtro,setpoint,kp,ki,kd);
		}


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	ucFlag = 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
