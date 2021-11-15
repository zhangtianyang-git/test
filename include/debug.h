
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "libscommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <assert.h>

#include <ptlib.h>

////////////////////////////////////////////
// debug options
////////////////////////////////////////////
//TODO - move to CLog
typedef enum {
	LOG_EMERG = 0,  /* system is unusable */
	LOG_ALERT,  /* action must be taken immediately */
	LOG_CRIT,   /* critical conditions */
	LOG_ERR,    /* error conditions */
	LOG_WARNING,    /* warning conditions */
	//the msg just show once
	LOG_NOTICE, /* normal but significant condition */
	//the msg less frequently showed up
	LOG_INFO,   /* informational */
	//the msg frequently showed up
	LOG_DEBUG,  /* debug-level messages */
	LOG_MAX,
} LOG_LEVEL;

#define COL(x, info)  	"\033[" #x "m" #info "\033[0m"
#define RED(info)   	COL(31, info)
#define GREEN(info)  	COL(32, info)
#define YELLOW(info) 	COL(33, info)
#define BLUE(info)   	COL(34, info)
#define MAGENTA(info)	COL(35, info)
#define CYAN(info)   	COL(36, info)
#define WHITE(info)  	COL(00, info)

//#ifdef PTRACE
#if 1
#define ETRACE(level, args) \
	do {\
		if (level > LOG_WARNING) {\
			PTRACE(level, GREEN(level) ": " args);\
		} else {\
			PTRACE(level, RED(level) ": " args);\
		}\
	} while (0)
#else
#define ETRACE(level, args) \
	do {\
		if((print_level >= 1) && (level <= print_level))\
			printf(args);\
	} while(0)
#endif

//TODO - color it
//extern const char *level_name_to_string[];
//#define ETRACE(level, args) PTRACE(level, level_name_to_string[level] args);
//#define TEST(level)     (level>4)?32:31
//#define ETRACE(level, args) PTRACE(level, "\033[" "31" "m" #level "\033[0m " ##args)

#define name_to_str(name) (#name)

#define LOG_TEMP LOG_EMERG

#endif/*_DEBUG_H_*/

