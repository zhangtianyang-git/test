#ifndef _LOCAL_LOG_H_
#define _LOCAL_LOG_H_

#include <stdio.h>

/* 宏对定义的先后顺序没有要求 */
/* 当前文件定义的打印等级 */
#define LOCAL_PRINT_LEVEL	LOCAL_DEBUG

#define LOCAL_DEBUG		9
#define LOCAL_INFO		7
#define LOCAL_WARNING	5
#define LOCAL_ERROR		3
#define LOCAL_CRITICAL	1

/* 红色对应 CRITICAL 与 ERROR log
 * 黄色对应 WARNING log
 * 绿色对应 INFO log
 * 蓝色对应 DEBUG log
 */
#define LOCAL_RED		"\033[31;1m"
#define LOCAL_YELLOW	"\033[0;33m"
#define LOCAL_GREEN		"\033[0;32m"
#define LOCAL_BULE		"\033[0;34m"
#define LOCAL_END		"\033[0m"


#define print_debug(...) do { \
    SWITCH_PRINT(LOCAL_DEBUG, __VA_ARGS__); \
    } while(0)

#define print_info(...) do { \
    SWITCH_PRINT(LOCAL_INFO, __VA_ARGS__); \
    } while(0)

#define print_warning(...) do { \
    SWITCH_PRINT(LOCAL_WARNING, __VA_ARGS__); \
    } while(0)

#define print_error(...) do { \
    SWITCH_PRINT(LOCAL_ERROR, __VA_ARGS__); \
    } while(0)

#define print_critical(...) do { \
    SWITCH_PRINT(LOCAL_CRITICAL, __VA_ARGS__); \
    } while(0)

#define SWITCH_PRINT(fmt, ...) do { \
    if (fmt <= LOCAL_PRINT_LEVEL) { \
    switch (fmt) { \
    case LOCAL_DEBUG: \
    printf(LOCAL_BULE); \
    break; \
    case LOCAL_INFO: \
    printf(LOCAL_GREEN); \
    break; \
    case LOCAL_WARNING: \
    printf(LOCAL_YELLOW); \
    break; \
    case LOCAL_ERROR: \
    case LOCAL_CRITICAL: \
    printf(LOCAL_RED); \
    break; \
    default: \
    break;\
    } \
    printf(__VA_ARGS__); \
    printf(LOCAL_END); \
    } \
    } while(0)

#endif	//_LOCAL_LOG_H_
