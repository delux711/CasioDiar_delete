#include "serialPort_u1.h"
#include "fifo_buffer.h"
//#include <stdio.h>

typedef unsigned char uint8_t;
//#define BUFF_MAX  50U
#define BUFF_MAX  4U
#define BUFF_MAX_TEST  8U

//static void SPu1_receiveTest(uint8_t ch);
static volatile uint8_t SPu1_test;
static volatile uint8_t SPu1_test1;
static volatile uint8_t FIFO_test;
static volatile uint8_t FIFO_test1;
static volatile uint8_t SPu1_test2;

static uint8_t SPu1_pPlus(void);
static bool SPu1_pWriteCheck(void);
static unsigned char SPu1_SPbuff[BUFF_MAX];
static uint8_t SPu1_recBuff[BUFF_MAX];
static uint8_t SPu1_recBuffTest[BUFF_MAX_TEST];
static uint8_t SPu1_pWrite;
static uint8_t SPu1_pRead;
//static uint8_t SPu1_pRecEnd;
//static uint8_t SPu1_pRecStart;
static uint8_t SPu1_ucReadData;
static bool SPu1_bNewData = false;
static bool SPu1_bPause = false;

uint8_t SPu1_sendBuff(uint8_t *buff, uint8_t length) {
    uint8_t i;
    for(i = 0; i < length; i++) {
        SPu1_sendChar(buff[i]);
    }
    return 1;
}

uint8_t SPu1_sendString(uint8_t *buff) {
    uint8_t i = 0;
    uint8_t ch = buff[0];
    while(ch != '\0') {
        SPu1_sendChar(ch);
        ch = buff[++i];
    }
    return 1;
}

uint8_t SPu1_sendChar(uint8_t ch) {
   //GPIOB->ODR |= (1u << GPIO_ODR_OD6_Pos);
    /* Do your stuff here */
    /* SSPu1_pRead your custom byte */
    /* SSPu1_pRead byte to USART */
    //TM_USART_Putc(USART1, ch);
    //while(!u1_n());
    SPu1_SPbuff[SPu1_pPlus()] = ch;
    if(SPu1_bPause == false) {
        USART1->CR1 |= USART_CR1_TXEIE;
    }

    /* If everything is OK, you have to return character written */
    //GPIOB->ODR &= ~(1u << GPIO_ODR_OD6_Pos);
    return ch;
    /* If character is not correct, you can return EOF (-1) to stop writing */
    //return -1;
}

void USART1_IRQHandler(void) {
   uint32_t status;

   status = USART1->ISR;  /* Read the status register to clear the flags. */

    if((status & USART_ISR_RXNE) != 0u) {
       SPu1_ucReadData = USART1->RDR;
       SPu1_bNewData = true;
/*
       if(SPu1_pRecStart != (SPu1_pRecEnd + 1u)) {
            if((BUFF_MAX <= SPu1_pRecEnd) && (SPu1_pRecStart != 0u)) {
                SPu1_pRecEnd = 0u;
            }
            SPu1_recBuff[SPu1_pRecEnd++] = USART1->RDR;
       }
  */     
    } else if((status & USART_ISR_ORE_Msk) != 0u) {
       USART1->ICR = USART_ICR_ORECF_Msk;
    }
   if(SPu1_pRead != SPu1_pWrite) {
      USART1->TDR = SPu1_SPbuff[SPu1_pRead++];
      if(BUFF_MAX <= SPu1_pRead) {
         SPu1_pRead = 0;
      }
   } else {
      USART1->CR1 &= ~USART_CR1_TXEIE;
   }
}

void SPu1_pauseOn(void) {
    USART1->CR1 &= ~USART_CR1_TXEIE;
    SPu1_bPause = true;
}

void SPu1_pauseOff(void) {
    SPu1_bPause = false;
    USART1->CR1 |= USART_CR1_TXEIE;
}

static bool SPu1_pWriteCheck(void) {
   if((SPu1_pWrite != SPu1_pRead) || (~USART1->CR1 & USART_CR1_TXEIE)) {
      return true;
   }
   return false;
}

static uint8_t SPu1_pPlus(void) {
   uint8_t temp;
   while(!SPu1_pWriteCheck());
   temp = SPu1_pWrite;
   SPu1_pWrite++;
   if(BUFF_MAX <= SPu1_pWrite) {
       SPu1_pWrite = 0;
   }
   return temp;
}

void SPu1_init(void) {
    uint8_t i;
    /*
    SPu1_pRecEnd = 0u;
    SPu1_pRecStart = 0xFFu;*/
    SPu1_test=0xaa;
    SPu1_test1=0xbb;
    SPu1_test2=0xcc;
    FIFO_init(0u, SPu1_recBuff, BUFF_MAX);
    FIFO_init(1u, SPu1_recBuffTest, BUFF_MAX_TEST);
    SPu1_test = FIFO_getData();
    for(i = 0; i < BUFF_MAX; i++) {
        SPu1_recBuff[i] = 0x30u + i;
    }
    for(i = 0; i < BUFF_MAX; i++) {
        SPu1_recBuffTest[i] = 0x60u + i;
    }
    for(i = 0; i < 12; i++) {
        FIFO_changeConfig(0);
        FIFO_putData(i);
        FIFO_putData(i+0x10);
        FIFO_putData(i+0x20);
        FIFO_changeConfig(1);
        FIFO_putData(i+0xA0);
        FIFO_putData(i+0xB0);
        FIFO_putData(i+0xC0);
        FIFO_changeConfig(0);
        SPu1_test = FIFO_getData();
        (void)SPu1_test;
        SPu1_test1 = FIFO_getData();
        (void)SPu1_test1;
        FIFO_changeConfig(1);
        FIFO_test = FIFO_getData();
        (void)FIFO_test;
        FIFO_test1 = FIFO_getData();
        (void)FIFO_test1;
        //SPu1_test2 = FIFO_getData();
        //(void)SPu1_test2;
    }
    
    
   RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk;
   GPIOB->MODER &= ~(GPIO_MODER_MODE6_Msk);
   GPIOB->MODER |= (1U << GPIO_MODER_MODE6_Pos);
    GPIOB->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED6_Pos);
   
   SPu1_pWrite = 0u;//BUFF_MAX;
   SPu1_pRead = 0u;
   RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN_Msk;
   RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
   RCC->CR |= RCC_CR_HSION_Msk;
   while(!RCC->CR & RCC_CR_HSIRDY_Msk);
   RCC->CCIPR |= (2U << RCC_CCIPR_USART1SEL_Pos); // 10: HSI16 clock selected as USART1 clock
   //USART_TX -> PB6
   //USART_RX -> PB7
   GPIOB->MODER &= (~(GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk));
   GPIOB->MODER |= ((2U << GPIO_MODER_MODE6_Pos) | (2U << GPIO_MODER_MODE7_Pos));
   GPIOB->AFR[0] |= ((7U << GPIO_AFRL_AFSEL6_Pos) | (7U << GPIO_AFRL_AFSEL7_Pos));
   USART1->BRR = (16 * 1000000) / (96 * 100); 
   /* Enable the USART unit. */
   //USART1->CR1 = USART_CR1_UE;
   /* Set TE and RE bits */
   USART1->CR2 |= (2u << USART_CR2_STOP_Pos);    // set to 2 stop bits
     (void) USART1->ISR;  /* Read the status register to clear the flags. */
   USART1->CR1 |= USART_CR1_UE;
   USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);
   USART1->CR1 |= USART_CR1_RXNEIE;
     NVIC_EnableIRQ(USART1_IRQn);
   /* Set the global interrupt into enabled state. */
    //for(i = 0; i < 15; i++)
    //     SPu1_sendChar((uint8_t)(i+'A'));
}

bool SPu1_isNewData(void) {
   return SPu1_bNewData;
   //return SPu1_pRecStart != SPu1_pRecEnd;
}

uint8_t SPu1_getData(void) {
    SPu1_bNewData = false;
    return SPu1_ucReadData;
}
/*
uint8_t FIFO_getData(void) {
    //SPu1_bNewData = false;
    //return SPu1_ucReadData;
    uint8_t ret;
    ret = SPu1_pRecStart;
    if(SPu1_pRecStart != SPu1_pRecEnd) {
        SPu1_pRecStart++;
        if(BUFF_MAX <= SPu1_pRecStart) {
            SPu1_pRecStart = 0;
            if(BUFF_MAX <= SPu1_pRecEnd) {
                SPu1_pRecEnd = 0;
            }
        }
    } else {
        if(0u == SPu1_pRecStart) {
            ret = BUFF_MAX - 1U;
        } else {
            ret = SPu1_pRecStart - 1u;
        }
    }
    ret = SPu1_recBuff[ret];
    return ret;
}

void FIFO_putData(uint8_t ch) {
    if(SPu1_pRecStart != (SPu1_pRecEnd + 1u)) {
        if(BUFF_MAX <= SPu1_pRecEnd) {
            SPu1_pRecEnd = 0u;
        }
        SPu1_recBuff[SPu1_pRecEnd] = ch;
        SPu1_pRecEnd++;
    }
}
*/
