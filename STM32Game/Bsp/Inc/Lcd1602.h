#ifndef LCD1602_H
#define LCD1602_H

#include <stdbool.h>
#include <stdint.h>

#define LCD1602_COLUMNS 16U
#define LCD1602_ROWS     2U

bool Lcd1602Init(void);
bool Lcd1602IsReady(void);
void Lcd1602Clear(void);
void Lcd1602SetCursor(uint8_t column, uint8_t row);
void Lcd1602WriteString(const char *text);
void Lcd1602Printf(const char *format, ...);

#endif /* LCD1602_H */
