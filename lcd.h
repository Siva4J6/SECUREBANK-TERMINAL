/**
 * @file      lcd.h
 * @brief     LCD Driver Prototypes
 * @author    Shaik Abbas
 * @date      November 2026
 */

#ifndef LCD_H
#define LCD_H

#include "types.h"

void WriteLCD(u8 byte);
void CmdLCD(u8 cmd);
void InitLCD(void);
void CharLCD(u8 asciiVal);
void StrLCD(s8 *str);
void U32LCD(u32 n);
void S32LCD(s32 n);
void F32LCD(f32 fn, u8 nDP);
void BuildCGRAM(u8 *p, u8 nBytes);

#endif /* LCD_H */
