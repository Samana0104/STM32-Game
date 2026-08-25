#pragma once
#include "main.h"

typedef enum
{
    INFO = 0,
    WARNING,
    DANGER
} LogLevel;

#ifdef DEBUG
#define G_LOG(...) CommandLog(__VA_ARGS__)
#else
#define G_LOG(...) do { } while (0)
#endif

// USART2에서 입력된 문자열을 파싱하여 명령어를 처리하는 함수
void CheatInit(void);
void CheatUpdate(void);
void CheatFrameTick(void);
void ParseCommand(const char *cmd);
void CommandLog(LogLevel level, const char *args, ...);
