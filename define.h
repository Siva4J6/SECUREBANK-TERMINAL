/**
 * @file      define.h
 * @brief     Bitwise Macro Definitions
 * @author    Siva Reddy
 * @date      November 2026
 */

#ifndef DEFINE_H
#define DEFINE_H

#define READBIT(WORD, BIT) ((WORD >> BIT) & 1)
#define WRITEBIT(WORD, BIT, BITVAL)  WORD = ((WORD & ~(1 << BIT)) | (BITVAL << BIT))
#define WRITEBYTE(WORD, STARTBIT, BYTE) WORD = ((WORD & ~(0xFF << STARTBIT)) | (BYTE << STARTBIT))
#define READWRITEBIT(WORD, DBIT, SBIT) WORD = ((WORD & ~(1 << DBIT)) | (((WORD >> SBIT) & 1) << DBIT))
#define READWRITEBYTE(DWORD, SWORD, DBIT, SBIT) DWORD = ((DWORD & ~(1 << DBIT)) | (((SWORD >> SBIT) & 1) << DBIT))
#define READNIBBLE(WORD, STARTBIT) ((WORD >> STARTBIT) & 0x0F)
#define WRITENIBBLE(WORD, STARTBIT, NIBBLE) WORD = ((WORD & (u32)~0X0F << STARTBIT) | (NIBBLE << STARTBIT))

#endif /* DEFINE_H */
