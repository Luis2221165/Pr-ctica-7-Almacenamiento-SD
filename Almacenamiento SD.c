/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define BUFFER_SIZE 128
SPI_HandleTypeDef hspi1;
FATFS fs;
FIL fil;
FRESULT fres;
UINT bytesRead;
uint8_t respuesta[1];
char buffer[BUFFER_SIZE];  // Buffer para almacenar los datos leídos

/* USER CODE END PD */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// Función para transmitir datos por UART
void transmit_uart(char *string) {
    uint8_t len = strlen(string);
    HAL_UART_Transmit(&huart2, (uint8_t*)string, len, 200);
}

void manejar_archivo_sd(const char *filename) {
    FRESULT fres;      // Variable para el resultado de las operaciones en la SD
    FIL fil;           // Variable para el archivo
    char buffer[128];  // Buffer para almacenar los datos leídos
    UINT bytesRead;    // Variable para almacenar la cantidad de bytes leídos

    // 1. Montar la SD
    fres = f_mount(&fs, "", 0);
    if (fres == FR_OK) {
        transmit_uart("Tarjeta Micro SD montada con éxito!\n");
    } else {
        transmit_uart("Error al montar la tarjeta Micro SD!\n");
        return;
    }

    // 2. Abrir el archivo
    fres = f_open(&fil, filename, FA_READ);
    if (fres == FR_OK) {
        transmit_uart("Archivo abierto para lectura.\n");
    } else {
        transmit_uart("No se pudo abrir el archivo para lectura!\n");
        f_mount(NULL, "", 0);
        return;
    }

    // 3. Leer el archivo y enviar el contenido por UART
    do {
        fres = f_read(&fil, buffer, sizeof(buffer) - 1, &bytesRead);
        if (fres == FR_OK) {
            buffer[bytesRead] = '\0';  // Asegurarse de que el buffer está terminado
            transmit_uart(buffer);     // Enviar el contenido por UART
        } else {
            transmit_uart("Error al leer el archivo.\n");
            break;
        }
    } while (bytesRead > 0);

    // 4. Cerrar el archivo
    fres = f_close(&fil);
    if (fres == FR_OK) {
        transmit_uart("Archivo cerrado correctamente.\n");
    } else {
        transmit_uart("Error al cerrar el archivo.\n");
    }

    // 5. Desmontar la SD
    fres = f_mount(NULL, "", 0);
    if (fres == FR_OK) {
        transmit_uart("Tarjeta SD desmontada con éxito!\n");
    } else {
        transmit_uart("Error al desmontar la tarjeta SD!\n");
    }
}

// Función para listar archivos en la SD y enviar los nombres por UART
void list_files_and_send_uart(void) {
    FATFS fs;
    FILINFO fno;
    DIR dir;
    FRESULT fres;
    char buffer[BUFFER_SIZE];

    // 1. Montar la SD
    fres = f_mount(&fs, "", 0);
    if (fres == FR_OK) {
        transmit_uart("Micro SD card is mounted successfully!\n");
    } else {
        transmit_uart("Micro SD card mount error!\n");
        return;
    }

    // 2. Abrir el directorio raíz
    fres = f_opendir(&dir, "/");
    if (fres == FR_OK) {
        transmit_uart("Root directory opened.\n");
    } else {
        transmit_uart("Error opening root directory.\n");
        f_mount(NULL, "", 0);
        return;
    }

    // 3. Leer los archivos y enviar sus nombres por UART
    while (1) {
        fres = f_readdir(&dir, &fno);
        if (fres != FR_OK || fno.fname[0] == 0) {
            break;
        }
        if (fno.fattrib & AM_DIR) {
            snprintf(buffer, BUFFER_SIZE, "Directory: %s\n", fno.fname);
        } else {
            snprintf(buffer, BUFFER_SIZE, "File: %s\n", fno.fname);
        }
        transmit_uart(buffer);
    }

    // 4. Cerrar el directorio y desmontar la SD
    f_closedir(&dir);
    f_mount(NULL, "", 0);
}

/* USER CODE END 0 */

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART2_UART_Init();
    MX_FATFS_Init();

    while (1) {
        transmit_uart("Ingrese un numero:\n1:Ver archivos\n2:Seleccionar archivo\n");
        HAL_UART_Receive(&huart2, respuesta, 1, 10000);
        
        if (respuesta[0] == '1') {
            transmit_uart("Lista de archivos:\n");
            list_files_and_send_uart();
        }
        
        if (respuesta[0] == '2') {
            transmit_uart("Seleccione un archivo:\n1. ascii-Heart.txt\n2. ascii-penguin.txt\n3. ascii-creeper.txt\n");
            HAL_UART_Receive(&huart2, respuesta, 1, 10000);
            
            if (respuesta[0] == '1') {
                read_file_and_send_uart("ascii-Heart.txt");
            } else if (respuesta[0] == '2') {
                read_file_and_send_uart("ascii-penguin.txt");
            } else if (respuesta[0] == '3') {
                read_file_and_send_uart("ascii-creeper.txt");
            }
        }
        
        respuesta[0] = '0';  // Reset para la próxima lectura
    }
}

/* USER CODE BEGIN 6 */
/* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
/* USER CODE END 6 */

/* System Clock Configuration, SPI1, USART2, and other initialization functions */

/* USER CODE BEGIN 4 */
/* Aquí puedes agregar más funciones personalizadas si es necesario */
/* USER CODE END 4 */
