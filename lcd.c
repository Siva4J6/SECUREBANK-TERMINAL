/**
 * @file      lcd.c
 * @brief     16x2 Character LCD Driver (8-bit mode)
 * @hardware  NXP LPC2148
 * @author    Siva Reddy
 * @date      November 2026
 */

#include <LPC21xx.h>
#include "types.h"
#include "define.h"
#include "delay.h"
#include "lcd_defines.h"

/* Sends a single byte of data to the LCD hardware pins and triggers the enable pulse */
void WriteLCD(u8 byte) {
    WRITEBYTE(IOPIN0, LCD_DATA, byte);
    IOCLR0 = 1 << LCD_RW;
    IOSET0 = 1 << LCD_EN;
    delay_us(1);
    IOCLR0 = 1 << LCD_EN;
    delay_ms(2);				  
}

/* Sends a command instruction to the LCD control register */
void CmdLCD(u8 cmd) {
    IOCLR0 = 1 << LCD_RS;
    WriteLCD(cmd);
}

/* Initializes the LCD hardware into 8-bit mode and clears the display */
void InitLCD(void) {
    IODIR0 |= ((1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN));
    IODIR0 |= ((0xFF << LCD_DATA));

    delay_ms(15);
    CmdLCD(0x30);
    delay_ms(4);
    delay_us(100);
    CmdLCD(0x30);
    delay_us(100);
    CmdLCD(0x30);
    
    CmdLCD(MODE_8BIT_2LINE);
    CmdLCD(DSP_ON_CUR_OFF);
    CmdLCD(CLEAR_LCD);
    CmdLCD(SHIFT_CUR_RIGHT);
}

/* Prints a single ASCII character to the current cursor position on the LCD */
void CharLCD(u8 asciiVal) {
    IOSET0 = 1 << LCD_RS;
    WriteLCD(asciiVal);
}

/* Prints a continuous null-terminated string to the LCD */
void StrLCD(s8 *str) {
    while (*str)
        CharLCD(*str++);
}

/* Converts and prints an unsigned 32-bit integer to the LCD */
void U32LCD(u32 n) {
    u8 a[10];
    s32 i = 0;
    
    if (n == 0) {
        CharLCD('0');
    } else {
        while (n > 0) {
            a[i] = (n % 10) + 48;
            n /= 10;
            i++;
        }
        for (--i; i >= 0; i--)
            CharLCD(a[i]);
    }
}

/* Converts and prints a signed 32-bit integer to the LCD, handling negative signs */
void S32LCD(s32 n) {
    if (n < 0) {
        CharLCD('-');
        n = -n;
    }
    U32LCD(n);
}

/* Converts and prints a 32-bit floating-point number to the LCD with a specified decimal precision */
void F32LCD(f32 fn, u8 nDP) {
    u32 n; 
    s32 i;
    
    if (fn < 0.0) {
        CharLCD('-');
        fn = -fn;
    }
    
    n = fn;
    U32LCD(n);
    CharLCD('.');
    
    for (i = 0; i < nDP; i++) {
        fn = (fn - n) * 10;
        n = fn;
        CharLCD(n + 48);
    }
}

/* Loads custom character pixel data into the LCD's CGRAM memory */
void BuildCGRAM(u8 *p, u8 nBytes) {
    u8 i;
    CmdLCD(GOTO_CGRAM_START);
    IOSET0 = 1 << LCD_RS;
    for (i = 0; i < nBytes; i++) {
        WriteLCD(p[i]);
    }
    CmdLCD(GOTO_LINE1_POS0);
}
