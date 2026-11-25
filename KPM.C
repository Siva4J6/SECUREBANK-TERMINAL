/**
 * @file      kpm.c
 * @brief     4x4 Matrix Keypad Driver
 * @hardware  NXP LPC2148, 4x4 Membrane Keypad
 * @author    Siva Reddy
 * @date      November 2026
 * 
 * @note      Custom mapping for SecureBank ATM UI.
 *            Rows: P1.16-P1.19 | Cols: P1.20-P1.23
 */

#include "types.h"
#include "define.h"
#include "delay.h"
#include "KPM_defines.h"
#include "lcd.h"
#include "lcd_defines.h"
#include <lpc21xx.h>

u8 KPMLUT[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','*'},
    {'C','0','=','#'}
};

/* Initializes the keypad row pins as GPIO outputs */
void Init_KPM(void) {
    WRITENIBBLE(IODIR1, ROW0, 15);
}

/* Scans all columns to check if any key is currently pressed */
u32 colscan(void) {
    if (READNIBBLE(IOPIN1, COL0) < 15)
        return 0;
    else
        return 1;
}

/* Identifies exactly which column contains the pressed key */
u32 colcheck(void) {
    u32 cno;
    for (cno = 0; cno < 4; cno++) {
        if ((READBIT(IOPIN1, (cno + COL0))) == 0)
            break;
    }
    return cno;
}

/* Identifies exactly which row contains the pressed key */
u32 rowcheck(void) {
    u32 rno;
    for (rno = 0; rno < 4; rno++) {
        WRITENIBBLE(IOPIN1, ROW0, (~(1 << rno)));
        if (colscan() == 0)
            break;
    }
    IOCLR1 = 15 << ROW0;
    return rno;
}

/* Blocking function that waits for a key press and returns the mapped character */
u32 keyscan(void) {
    u32 rno, cno, key;
    while (colscan());
    rno = rowcheck();
    cno = colcheck();
    key = KPMLUT[rno][cno];
    while (!colscan());
    delay_ms(400);
    return key;
}

/* Reads a sequence of numeric key presses from the user until Enter (#) is pressed */
static u32 ReadNumMode(u32 mask_digits) {
    u8 key;
    u32 num = 0, pos = 0;

    while (1) {
        CmdLCD(DSP_ON_CUR_BLK);
        key = keyscan();

        if (key == '*' && pos == 0) {
            CmdLCD(DSP_ON_CUR_OFF);
            return (u32)-1;
        } 
        else if (key >= '0' && key <= '9') {
            pos++;
            num = (num * 10) + (key - 48);
            if (mask_digits)
                CharLCD('*');
            else
                CharLCD(key);
        } 
        else if (key == 'C') {
            if (pos > 0) {
                num = num / 10;
                pos--;
                CmdLCD(0x10);
                CharLCD(' ');
                CmdLCD(0x10);
            }
        } 
        else if (key == '#') {
            break;
        }

        if (pos > 6)
            break;
            
        delay_ms(500);
    }
    CmdLCD(DSP_ON_CUR_OFF);
    return num;
}

/* Reads a number from the keypad and displays the actual digits on the LCD */
u32 ReadNum(void) {
    return ReadNumMode(0); 
}

/* Reads a number from the keypad but displays asterisks (*) for PIN security */
u32 ReadNumMasked(void) {
    return ReadNumMode(1); 
}
