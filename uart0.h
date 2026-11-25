/**
 * @file      uart0.h
 * @brief     UART0 PC Communication Prototypes
 * @author    Siva Reddy
 * @date      November 2026
 *
 * @note      Polling-based interface for request/reply protocol with server.
 */

#ifndef UART0_H
#define UART0_H

void UART0_Init(void);
void UART0_SendChar(char c);
void UART0_SendString(const char *str);
int UART0_ReceiveString(char *buf, int buf_size, unsigned long timeout_loops);

#endif /* UART0_H */
