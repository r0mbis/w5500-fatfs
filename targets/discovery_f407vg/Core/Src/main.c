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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "crc.h"
#include "usart.h"
#include "dma.h"
#include "spi.h"
#include "sys_log.h"
#include "dhcp.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "w5500_driver.h"
#include "server_device.h"
#include <stdio.h>
#include "ftpd.h"
#include "fatfs_add.h"
#include "SPI_SDcard.h"
#include "fatfs_file_handler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHCP_TASK_STACK_SIZE 2048
#define DEVICE_TASK_STACK_SIZE 2048
#define CONTROL_TASK_STACK_SIZE 1024
#define BLINK_TASK_STACK_SIZE 512
#define SDINFO_TASK_STACK_SIZE 512
#define SDMANAGER_TASK_STACK_SIZE 512
#define PING_TASK_STACK_SIZE 1024

#define DHCP_TASK_PRIORITY 5
#define DEVICE_TASK_PRIORITY 3
#define CONTROL_TASK_PRIORITY 2
#define BLINK_TASK_PRIORITY 2
#define PING_TASK_PRIORITY 2
#define SDINFO_TASK_PRIORITY 4
#define SDMANAGER_TASK_PRIORITY 4

#define DHCP_RETRY_COUNT  5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TaskHandle_t dhcp_handle;
TaskHandle_t device_handle;
TaskHandle_t control_handle;
TaskHandle_t blink_handle;
TaskHandle_t ping_handle;
TaskHandle_t sdinfo_handle;
TaskHandle_t sdmanager_handle;

SemaphoreHandle_t sd_mutex; 

device_data_t device_data;
extern wiz_NetInfo network_info;
extern volatile bool dhcp_get_ip;

extern StoreDisk_t sDisk[2];

uint32_t m_sec_cnt;

uint8_t ftp_buf[1024] = {0,};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void dhcp_task(void *params);
void device_task(void *params);
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
  //HAL_InitTick(5U);
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  //SPI_SD_Init();
  /* USER CODE BEGIN 2 */
  sys_log_init();
  FH_mount(&sDisk[0]);
  InitW5500();
  device_data.ip_assigned_sem = xSemaphoreCreateCounting((UBaseType_t)0xFFFFFFFF, (UBaseType_t)0U);
  //device_data.ping_sem = xSemaphoreCreateBinary();
  device_data.send_queue = xQueueCreate((UBaseType_t)10, sizeof(message_t));
  device_data.receive_queue = xQueueCreate((UBaseType_t)10, sizeof(message_t));
  device_data.blink_queue = xQueueCreate(10, sizeof(uint32_t));
  device_data.events = xEventGroupCreate();
  //device_data.server_run = false;

  sd_mutex = xSemaphoreCreateMutex();

  log_i(ETH_TAG, "Creating tasks ...\r\n");
  xTaskCreate(dhcp_task, "DHCP_Task", DHCP_TASK_STACK_SIZE, (void*)&device_data, DHCP_TASK_PRIORITY, &dhcp_handle);
  xTaskCreate(device_task, "Device_Task", DEVICE_TASK_STACK_SIZE, (void*)&device_data, DEVICE_TASK_PRIORITY, &device_handle);
  xTaskCreate(blink_task, "Blink_Task", BLINK_TASK_STACK_SIZE, (void*)&device_data, BLINK_TASK_PRIORITY, &blink_handle);
  //xTaskCreate(control_task, "Control_Task", CONTROL_TASK_STACK_SIZE, (void*)&device_data, CONTROL_TASK_PRIORITY, &control_handle);
  //xTaskCreate(ping_task, "Ping_Task", PING_TASK_STACK_SIZE, (void*)&device_data, PING_TASK_PRIORITY, &ping_handle);
  //xTaskCreate(sdinfo_task, "SDinfo_Task", SDINFO_TASK_STACK_SIZE, (void*)sDisk, SDINFO_TASK_PRIORITY, &sdinfo_handle);
  //xTaskCreate(sdmanager_task, "SDmanager_Task", SDMANAGER_TASK_STACK_SIZE, NULL, SDMANAGER_TASK_PRIORITY, &sdmanager_handle);

  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    ;
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/*
*/
  void dhcp_task(void *params)
  {
    device_data_t *device_data_ptr = (device_data_t*)params;
    log_i(ETH_TAG, "DHCP task ...\r\n");
    int retval;
    uint8_t link;
    uint8_t dhcp_retry = 0;
    if (network_info.dhcp == NETINFO_DHCP)  // DHCP
    {
      dhcp_init(device_data_ptr->events);
    }
    else  // static
    {
      network_initialize(network_info);

      print_network_information(network_info);
      while(1)
      {
        vTaskDelay(1000 * 1000);
      }
    }

    while(1)
    {
      link = wizphy_getphylink();
      if (link == PHY_LINK_OFF)
      {
        log_i(ETH_TAG, "PHY_LINK_OFF\r\n");
        xEventGroupClearBits(device_data_ptr->events, EVENT_SERVER_RUN);
        //device_data.server_run = false;

        DHCP_stop();
        while (1)
        {
          link = wizphy_getphylink();

          if (link == PHY_LINK_OFF)
          {
            dhcp_init(device_data_ptr->events);
            dhcp_retry = 0;
            break;
          }
          vTaskDelay(1000);
        }
      }

      retval = DHCP_run();
      if (retval == DHCP_IP_LEASED)
      {
        if (!((xEventGroupGetBits(device_data_ptr->events) & EVENT_DHCP_IP_RECEIVED) ==  EVENT_DHCP_IP_RECEIVED))              // (dhcp_get_ip == false)
        {
          dhcp_retry = 0;
          log_i(ETH_TAG, "DHCP success\r\n");
          xEventGroupSetBits(device_data_ptr->events, EVENT_DHCP_IP_RECEIVED | EVENT_SERVER_RUN);
          //dhcp_get_ip = true;

          //device_data_ptr->server_run = true;
          HAL_GPIO_WritePin(LED_PORT, LED_BLUE, GPIO_PIN_SET);
          xSemaphoreGive(device_data_ptr->ip_assigned_sem);
        }

      }
      else if (retval == DHCP_FAILED)
      {
        xEventGroupClearBits(device_data_ptr->events, EVENT_DHCP_IP_RECEIVED);
        //dhcp_get_ip = false;
        dhcp_retry++;
        if (dhcp_retry <= DHCP_RETRY_COUNT)
        {
          log_i(ETH_TAG, "DHCP timeout occurred and retry %d\r\n", dhcp_retry);
        }
      }
      if (dhcp_retry > DHCP_RETRY_COUNT)
      {
        log_i(ETH_TAG, "DHCP failed\r\n");
        DHCP_stop();
        HAL_GPIO_WritePin(LED_PORT, LED_BLUE, GPIO_PIN_RESET);
        while (1)
        {
          vTaskDelay(1000 * 1000);
        }
      }

      vTaskDelay(20);
    }

  }

  void device_task(void *params)
  {
    device_data_t *device_data_ptr = (device_data_t*)params;
    socket_data_t socket_data[LISTENING_SOCKETS]; 
    socket_data_udp_t socket_data_udp[UDP_SOCKETS];

    while(1)
    {
      log_i(ETH_TAG, "Tcp device waiting for ip...\r\n");
      xSemaphoreTake(device_data_ptr->ip_assigned_sem, portMAX_DELAY);
      log_i(ETH_TAG, "IP Assigned, starting tcp device .\r\n");
      
      ftpd_init(network_info.ip);

      //Initialise socket data
      for (uint8_t i = 0; i < LISTENING_SOCKETS; i++)
      {
        socket_data[i].socket_id = i + 2;
        socket_data[i].listening_port = LISTENING_PORT;
        socket_data[i].socket_open = false;
        socket_data[i].receive_size = 0;
        socket_data[i].send_size = 0;

        socket(socket_data[i].socket_id, Sn_MR_TCP, socket_data[i].listening_port, 0x0);
        server_loop(&socket_data[i]);
      }
      for (uint8_t i = 0; i < UDP_SOCKETS; i++)
      {
        socket_data_udp[i].socket_id = LISTENING_SOCKETS + i + 2;
        socket_data_udp[i].sourceport = UDP_PORT;
        //socket(socket_data_udp[i].socket_id, Sn_MR_UDP, socket_data_udp[i].sourceport, 0x0);
        udp_loop(&socket_data_udp[i]);
      }

      while(xEventGroupWaitBits(device_data_ptr->events, EVENT_SERVER_RUN | EVENT_DHCP_IP_RECEIVED, pdFALSE, pdTRUE, pdMS_TO_TICKS(1)) == (EVENT_SERVER_RUN | EVENT_DHCP_IP_RECEIVED))                      // (device_data_ptr->server_run)
      {
        message_t send_message;
        send_message.message_type = NO_MESSAGE;
        send_message.value = 0;
        xQueueReceive(device_data_ptr->send_queue, (void*)&send_message, (TickType_t)100);

        for(uint8_t i = 0; i < LISTENING_SOCKETS; i++)
        {
          if (socket_data[i].socket_open == true)
          {
            if (send_message.client == socket_data[i].socket_id)
            {
              if (send_message.message_type != NO_MESSAGE)
              {
                if (send_message.message_type == MSG_BLINK)
                {
                  socket_data[i].send_size = sprintf((char*)socket_data[i].send_buffer, "Blink%d#", send_message.value);
                }
                else if (send_message.message_type == MSG_SET)
                {
                  socket_data[i].send_size = sprintf((char*)socket_data[i].send_buffer, "Set%d#", send_message.value);
                }
              }
            }

            if (socket_data[i].send_size == 0)
            {
              uint32_t time = HAL_GetTick();
              uint32_t last_command_send_time = (time - socket_data[i].last_command_send);
              if (last_command_send_time > (KEEP_ALIVE_SECONDS * 1000))
              {
                log_i(ETH_TAG, "[%d]: Sending heartbeat.", i);
                socket_data[i].send_size = sprintf((char*)socket_data[i].send_buffer, "HB%d#", send_message.value);
              }
            }
          }

          server_loop(&socket_data[i]);
          message_t received_message;
          //Check for received TCP messages
          while (handle_receive_bufffer(&socket_data[i], &received_message))
          {
            if ((received_message.message_type != NO_MESSAGE) && (received_message.message_type != MSG_KEEPALIVE))
            {
              log_i(ETH_TAG, "Message received from tcp client: %d, message_type: %d\r\n", received_message.client, received_message.message_type);
              if (xQueueSend(device_data_ptr->receive_queue, (void*)&received_message, 100) != pdTRUE)
              {
                log_i(ETH_TAG, "\nUnable to put message on receive_queue.\r\n");
              }
            }
          }
        }
        for (uint8_t i = 0; i < UDP_SOCKETS; i++)
        {
          udp_loop(&socket_data_udp[i]);
        }
        ftpd_run(ftp_buf);

      }
      log_i(ETH_TAG, "\nTcp server stopping\r\n");
    }
  }
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  // HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  // HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  // /* SysTick_IRQn interrupt configuration */
  // HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();

    m_sec_cnt++;
    if (m_sec_cnt > 999)
    {
      m_sec_cnt = 0;
      DHCP_time_handler();
    }
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/* USER CODE BEGIN 4 */

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
