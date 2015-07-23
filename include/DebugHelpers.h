
#pragma once

#include <stdarg.h>
#include "logging.h"

unsigned long GetTickCount();
unsigned long GetuS();

void DbgOut(int prio, const char *tag, const char *szLog);
void DbgOut1(int prio, const char *tag, const char * __restrict fmt, ...);
int  vDbgOut1(int prio, const char *tag, const char * __restrict fmt, va_list ap);

void ShowI2C(bool write, unsigned char i2c_addr, unsigned char reg, const unsigned char *buf, unsigned char len);

/** Converts time in milliseconds to a string in seconds.
    @param time - The this that is to be converted into a string.
    Calling @c time_str(1234) will return a string containing "1.234".
	Never call it twice since in a call since we use a static buffer!
	Example:
		printf("%s, %s", time_str(now), time_str(later));
	Will print the same value for both now and later.

*/
inline const char *time_str(unsigned long now)
{

	static char buffer[30];
	sprintf(buffer, "%u.%03u", now / 1000, now % 1000);
	return buffer;
}

#include <pthread.h>
/** Converts current in microseconds to a string in seconds.
    Calling @c now_str() will return a string containing "1.234.001".
*/
inline const char *now_str()
{
	static char buffernow[30];
 	struct timespec ts;
 	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
	unsigned long ms = ts.tv_nsec / (1000*1000);
	unsigned long us = (ts.tv_nsec / 1000) % 1000;
	sprintf(buffernow, "%u.%03u.%03u", ts.tv_sec, ms, us);
	return buffernow;
}

#ifdef NO_DEBUG
#define DBG_INIT()
#define DBG_DEINIT()
#define DBG_OUT(...)	((void)0)
#define DBG_OUT1(...)	((void)0)
#define DBG_I2C(...)	((void)0)
#define DBG_FUNC()		((void)0)

#undef  USE_HWTRACE
#else

#define DBG_INIT()
#define DBG_DEINIT()
#define DBG_OUT(x)					DbgOut(PRINT_LEVEL, LOG_TAG, x)
#define DBG_OUT1(format, arg...) 	DbgOut1(PRINT_LEVEL, LOG_TAG, format, ## arg)

//#ifdef __GNUC__
#define DBG_FUNC()					DBG_OUT(__PRETTY_FUNCTION__)
//#define DBG_FUNC()				DBG_OUT(__FUNCSIG__)	// for MSVC
//#else
//#define DBG_FUNC()					DBG_OUT(__FUNCTION__)
//#endif

#define SHOWERRORS 1
// PRINT_LEVEL above is just a dummy yet

#ifdef USE_I2CDEBUG
#define DBG_I2C			ShowI2C
#else
#define DBG_I2C(...)	((void)0)
#endif

#endif

// for turning certain line temporarily off
#define _DBG_OUT(...)	((void)0)
#define _DBG_OUT1(...)	((void)0)

#ifdef SHOWERRORS
#define DBG_ERR(x)			DbgOut(PRINT_LEVEL, LOG_TAG, x)
#define DBG_ERR1(format, arg...) 	DbgOut1(PRINT_LEVEL, LOG_TAG, format, ## arg)
#else
#define DBG_ERR(...)	((void)0)
#define DBG_ERR1(...)	((void)0)
#endif

///////////////////////////////////////////////////////////
#ifdef USE_TIMEDEBUG
void ResetTime();
void PrintTime(const char* szText);
#define RESET_TIME()	ResetTime()
#define PRINT_TIME(s)	PrintTime(s)
#else 	// TIME_DEBUG
#define RESET_TIME()
#define PRINT_TIME(s)
#endif   // TIME_DEBUG


///////////////////////////////////////////////////////////
//#define USE_V4L2DEBUG
#ifdef USE_V4L2DEBUG
void ShowV4L2Buffer(const char *pText, const struct v4l2_buffer *pBuffer);
#define DBG_V4L2_Buffer(b)	ShowV4L2Buffer(__FUNCTION__, b)
#else
#define DBG_V4L2_Buffer(b)
#endif

///////////////////////////////////////////////////////////
#include "ImageDebugInterface.h"

///////////////////////////////////////////////////////////
// Message Sequence Chart compatible log entries
#ifndef MSCGEN
#define MSCGEN 0
#endif

#include "mscgen.h"


///////////////////////////////////////////////////////////
// Writing to dmesg

void DmesgOpen();
void DmesgClose();
void DmesgOut(const char *szLog);
void DmesgOut1(const char *psz, ...);


#ifdef NO_DEBUG
#define DMSG_INIT()	((void)0)
#define DMSG_DEINIT()	((void)0)
#define DMSG_OUT(...)	((void)0)
#define DMSG_OUT1(...)((void)0)
#else
#define DMSG_INIT()					DmesgOpen()
#define DMSG_DEINIT()					DmesgClose()
#define DMSG_OUT(x)					DmesgOut(x)
#define DMSG_OUT1(format, arg...) 	DmesgOut1(format, ## arg)
#define DMSG_FUNC()					DmesgOut(__FUNCTION__) //  __FILE__ __LINE__)
#endif
