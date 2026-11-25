/**
 * @file      uart0.c
 * @brief     UART0 Driver for PC/Server Communication
 * @hardware  NXP LPC2148 (UART0 on P0.0 and P0.1)
 * @author    Shaik Abbas
 * @date      November 2026
 * 
 * @note      Configured for 9600 Baud, 8 Data Bits, 1 Stop Bit, No Parity (8N1).
 */

#include <lpc21xx.h>
#include "uart0.h"

#define DLAB_BIT           7
#define WORD_LEN_SEL_BITS  0
#define _8BIT              3
#define U0LSR_RDR_BIT      0   
#define U0LSR_THRE_BIT     5   

/* Initializes UART0 pins and sets the baud rate divisor for PC communication */
void UART0_Init(void) {
    PINSEL0 &= ~0x0000000F;
    PINSEL0 |=  0x00000005;

    U0LCR = ((1 << DLAB_BIT) | (_8BIT << WORD_LEN_SEL_BITS));
    U0DLL = 97; 
    U0DLM = 0;
    U0LCR &= ~(1 << DLAB_BIT);
}

/* Transmits a single character to the PC via UART0 */
void UART0_SendChar(char c) {
    while (!(U0LSR & (1 << U0LSR_THRE_BIT)));
    U0THR = c;
}

/* Transmits a null-terminated string to the PC via UART0 */
void UART0_SendString(const char *str) {
    while (*str != '\0') {
        UART0_SendChar(*str);
        str++;
    }
}

/* Blocks and reads characters from the PC into a buffer until a '$' terminator is received or a timeout occurs */
int UART0_ReceiveString(char *buf, int buf_size, unsigned long timeout_loops) {
    int idx = 0;
    unsigned long waited;
    char c;

    while (idx < buf_size - 1) {
        waited = 0;
        
        while (!(U0LSR & (1 << U0LSR_RDR_BIT))) {
            waited++;
            if (waited >= timeout_loops)
                return -1; 
        }

        c = (char) U0RBR;

        if (c == '$') break; 

        buf[idx++] = c;
    }

    buf[idx] = '\0';
    return idx;
}
