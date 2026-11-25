/**
 * @file      delay.c
 * @brief     Software Delay Loops
 * @hardware  NXP LPC2148
 * @author    Siva Reddy
 * @date      November 2026
 */

#include <lpc21xx.h>

/* Halts processor execution for the specified number of milliseconds */
void delay_ms(unsigned int data) {
    data *= 12000;
    while (data--);
}

/* Halts processor execution for the specified number of microseconds */
void delay_us(unsigned int data) {
    data *= 12;
    while (data--);
}

/* Halts processor execution for the specified number of seconds */
void delay_s(unsigned int data) {
    data *= 12000000;
    while (data--);
}
