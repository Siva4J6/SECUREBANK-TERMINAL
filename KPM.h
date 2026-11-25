/**
 * @file      KPM.h
 * @brief     4x4 Matrix Keypad Prototypes
 * @author    Siva reddy
 * @date      November 2026
 */

#ifndef KEYPAD_H
#define KEYPAD_H

#include "types.h"

void Init_KPM(void);
u32 colscan(void);
u32 rowcheck(void);
u32 colcheck(void);
u32 keyscan(void);
u32 ReadNum(void);
u32 ReadNumMasked(void);

#endif /* KEYPAD_H */
