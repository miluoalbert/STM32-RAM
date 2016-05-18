//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

/* Includes ------------------------------------------------------------------*/
#include "main.h"

// ----------------------------------------------------------------------------
//
//

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define BUFFER_SIZE         ((uint32_t)0x0001)
#define WRITE_READ_ADDR     ((uint32_t)0x0000)    //800)
#define REFRESH_COUNT       ((uint32_t)0x0569)   /* SDRAM refresh counter (90MHz SD clock) */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* SDRAM handler declaration */
SDRAM_HandleTypeDef hsdram;
FMC_SDRAM_TimingTypeDef SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;
/* UART handler declaration */
UART_HandleTypeDef UartHandle;

/* Read/Write Buffers */
uint32_t aTxBuffer;
uint32_t aRxBuffer;
uint32_t StartTime;
uint32_t EndTime;

/* Read/Write Datas */
float aRxData;
float aOutPutData;


/* Status variables */
__IO uint32_t uwWriteReadStatus = 0;

/* Counter index */
uint32_t uwIndex = 0;



/* Private function prototypes -----------------------------------------------*/
void tled_init(void);
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
void SDRAM_Refresh(void);
// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.
  tled_init();

  /*##Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART1 configured as follow:
        - Word Length = 8 Bits
        - Stop Bit = One Stop bit
        - Parity = None
        - BaudRate = 9600 baud
        - Hardware flow control disabled (RTS and CTS signals) */
    UartHandle.Instance          = USARTx;

    UartHandle.Init.BaudRate     = 9600;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&UartHandle) != HAL_OK)
    {
      while(1);
    }




  /* SDRAM device configuration */
  hsdram.Instance = FMC_SDRAM_DEVICE;

  /* Timing configuration for 90 MHz of SD clock frequency (180MHz/2) */
  /* TMRD: 2 Clock cycles */
  SDRAM_Timing.LoadToActiveDelay    = 2;
  /* TXSR: min=70ns (6x11.90ns) */
  SDRAM_Timing.ExitSelfRefreshDelay = 7;
  /* TRAS: min=42ns (4x11.90ns) max=120k (ns) */
  SDRAM_Timing.SelfRefreshTime      = 4;
  /* TRC:  min=63 (6x11.90ns) */
  SDRAM_Timing.RowCycleDelay        = 7;
  /* TWR:  2 Clock cycles */
  SDRAM_Timing.WriteRecoveryTime    = 2;
  /* TRP:  15ns => 2x11.90ns */
  SDRAM_Timing.RPDelay              = 2;
  /* TRCD: 15ns => 2x11.90ns */
  SDRAM_Timing.RCDDelay             = 2;

  hsdram.Init.SDBank             = FMC_SDRAM_BANK2;
  hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

  /* Initialize the SDRAM controller */
  HAL_SDRAM_Init(&hsdram, &SDRAM_Timing);
  /* Program the SDRAM external device */
  SDRAM_Initialization_Sequence(&hsdram, &command);


  aTxBuffer=0x11223344;



    if(HAL_UART_Receive_IT(&UartHandle, (uint8_t*)&aTxBuffer, 4) != HAL_OK)
      {
          while(1);
      }
  // Infinite loop
  while (1)
    {
       //BrainSignal Caculation Program
      //------------test----------------
      //*(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex) = aTxBuffer;
      //aRxBuffer = *(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex);

      StartTime= HAL_GetTick();
      SDRAM_Refresh();
      EndTime=HAL_GetTick();

      HAL_UART_Transmit(&UartHandle, (uint8_t*)&StartTime, 4, 5000);
      HAL_UART_Transmit(&UartHandle, (uint8_t*)&EndTime, 4, 5000);
      HAL_Delay(5000);
      BSP_LED_Toggle(LED3);

    }
}

void tled_init(void)
{
  /* Configure LED3 and LED4 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
}

/**
  * @brief  Perform the SDRAM exernal memory inialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
  __IO uint32_t tmpmrd =0;
  /* Configure a clock configuration enable command */
  Command->CommandMode       = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget     = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber   = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /*  Insert 100 ms delay */
  HAL_Delay(100);

  /* Configure a PALL (precharge all) command */
  Command->CommandMode       = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget       = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber   = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Configure a Auto-Refresh command */
  Command->CommandMode       = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget     = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber   = 4;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget     = FMC_SDRAM_CMD_TARGET_BANK2;
  Command->AutoRefreshNumber   = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

void SDRAM_Refresh(void)
{
  uint32_t prim;
  prim = __get_PRIMASK();
  //disable interrupt
  __disable_irq();

  /*
   uint32_t index;
  for(index=0;index<0x80000;index++)
  {
      (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex) = (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 0x200000+ 4*uwIndex);
  }
   */
  memcpy((SDRAM_BANK_ADDR + WRITE_READ_ADDR), (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 0x400000 ), 0x400000);

  if (!prim)
  {
    __enable_irq();
  }
}




/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)  //current UART
    {

      //Data processing
      if(1)   //parity check
      {

      }
      else
      {

      }

        //if(Rx_data[0]=='a')
      BSP_LED_Toggle(LED4);
      /* Write data to the SDRAM memory
      if(uwIndex<2097151)
          {
            uwIndex++;
          }
          else
          {
            uwIndex=0;
          }*/

    *(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex) = aTxBuffer;
    HAL_UART_Receive_IT(&UartHandle,(uint8_t*)&aTxBuffer, 4); //activate UART receive interrupt every time
    }

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
