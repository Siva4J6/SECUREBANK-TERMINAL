/**
 * @file      rfid.c
 * @brief     EM18 RFID Reader Interrupt Driver
 * @hardware  NXP LPC2148 (UART1), EM18 RFID Module
 * @author    Siva Reddy
 * @date      November 2026
 * 
 * @note      Uses UART1 RX interrupt to asynchronously capture 8-digit RFID tags.
 */

#include <lpc21xx.h>

typedef unsigned char  u8;
typedef unsigned int   u32;

#define DLAB_BIT           7
#define WORD_LEN_SEL_BITS  0
#define _8BIT              3
#define RFID_FRAME_LEN     8   

volatile unsigned char rfid_buffer[16];
volatile u8 rfid_index = 0;
volatile u8 rfid_ready = 0;

void UART1_ISR(void) __irq;

/* Initializes UART1 for the EM18 reader and configures the Vectored Interrupt Controller (VIC) */
void UART1_Init(void) {
    PINSEL0 &= ~0x000F0000;
    PINSEL0 |=  0x00050000;      

    U1LCR = ((1 << DLAB_BIT) | (_8BIT << WORD_LEN_SEL_BITS));
    U1DLL = 97;                  
    U1DLM = 0;
    U1LCR &= ~(1 << DLAB_BIT);

    U1IER = 0x01;                

    VICVectAddr6  = (u32)UART1_ISR;  
    VICVectCntl6  = 0x20 | 7;        
    VICIntEnable  = (1 << 7);        
}

/* Interrupt Service Routine (ISR) that parses incoming bytes into a complete RFID tag string */
void UART1_ISR(void) __irq {
    u8 iir = U1IIR & 0x0E;

    if (iir == 0x04 || iir == 0x0C) {
        u8 data = U1RBR;

        if (data == 0x02) {
            rfid_index = 0;
            rfid_ready = 0;
        } 
        else if (data == 0x03) {
            if (rfid_index == RFID_FRAME_LEN) {
                rfid_buffer[rfid_index] = '\0';
                rfid_ready = 1;
            }
            rfid_index = 0;             
        } 
        else if (rfid_index < 12) {
            rfid_buffer[rfid_index++] = data;
        }
    }
    VICVectAddr = 0x00;                
}
