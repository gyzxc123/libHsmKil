//! \file
#define LOG_TAG "Kil"

#include "CommonInclude.h"
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>
#include "logging.h"

///////////////////////////////////////////////////////////////////////////////////////////
DBGAPI unsigned long GetTickCount()
{
 	struct timespec ts;
 	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
 	unsigned int ret(ts.tv_sec);
 	return ret * 1000 + ts.tv_nsec / 1000000;
}
///////////////////////////////////////////////////////////////////////////////////////////
DBGAPI unsigned long GetuS()
{
 	struct timespec ts;
 	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
 	unsigned int ret(ts.tv_sec);
 	return ret * 1000000 + (ts.tv_nsec / 1000);
}

//////////////////////////////////////////////////////////////////////

static void ts_diff(timespec start, timespec end, timespec &diff)
{
	const long Second = 1000000000L;
	if ( ((end.tv_nsec-start.tv_nsec)<0) || ((end.tv_nsec-start.tv_nsec)>Second) )
	{
		diff.tv_sec = end.tv_sec-start.tv_sec-1;
		diff.tv_nsec = Second+end.tv_nsec-start.tv_nsec;
	}
	else
	{
		diff.tv_sec = end.tv_sec-start.tv_sec;
		diff.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
}

static struct timespec old_ts =
{
	0,
	0,
};

#ifdef USING_PRINTF
static const bool gs_bPrintTime=true;
#define LOG_PRINT_TAG(tag)			printf("%s: ", tag)
#define LOG_PUTS(prio, tag, szLog)		puts(szLog)
#define LOG_PRINT(prio, tag, fmt, arg...)	printf(fmt, ## arg)
#define LOG_VPRINT(prio, tag, fmt, pArgs)	vprintf(fmt, pArgs)
#else
static const bool gs_bPrintTime=false;	// does not look good in logcat
#define LOG_PRINT_TAG(tag)			((void)0)
#define LOG_PUTS(prio, tag, szLog)	__android_log_write(prio, tag, szLog)
#define LOG_PRINT(prio, tag, fmt, arg...)	__android_log_print(prio, tag, fmt, ## arg)
#define LOG_VPRINT(prio, tag, fmt, pArgs)	__android_log_vprint(prio, tag, fmt, pArgs)
#endif

#ifdef USE_TIMEDEBUG
static void PrintMicroSec(int prio, const char *tag)
{
	LOG_PRINT_TAG(tag);

 	if(gs_bPrintTime)
 	{
		struct timespec ts;
		struct timespec diff;
		clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
		ts_diff(old_ts, ts, diff);
		old_ts.tv_sec=ts.tv_sec;
		old_ts.tv_nsec=ts.tv_nsec;

		LOG_PRINT(prio, tag, "%li.%06li-%2li.%06li: ", ts.tv_sec, ts.tv_nsec / 1000, diff.tv_sec, diff.tv_nsec / 1000);
 	}
}
#else
#define PrintMicroSec(prio, tag)	LOG_PRINT_TAG(tag)
#endif

static pthread_mutex_t LogProtect = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t LogProtectLevel1 = PTHREAD_MUTEX_INITIALIZER;

DBGAPI void DbgOut(int prio, const char *tag, const char *szLog)
{
	pthread_mutex_lock(&LogProtect);
	PrintMicroSec(prio, tag);
	LOG_PUTS(prio, tag, szLog);
	pthread_mutex_unlock(&LogProtect);
}

DBGAPI void DbgOut1(int prio, const char *tag, const char * __restrict fmt, ...)
{
	pthread_mutex_lock(&LogProtect);
	PrintMicroSec(prio, tag);
	va_list	pArgs;
	va_start( pArgs, fmt );
	LOG_VPRINT(prio, tag, fmt, pArgs);
	pthread_mutex_unlock(&LogProtect);
}

DBGAPI int vDbgOut1(int prio, const char *tag, const char * __restrict fmt, va_list ap)
{
	pthread_mutex_lock(&LogProtect);
	PrintMicroSec(prio, tag);
	int ret = LOG_VPRINT(prio, tag, fmt, ap);
	pthread_mutex_unlock(&LogProtect);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
static struct timeval tv;
static struct timeval start_tv;
static struct timeval old_tv;

/*
 * Make sure that the first argument is the more recent time, as otherwise
 * we'll get a weird negative time-diff back...
 *
 * Returns: the time difference in number of milliseconds.
 */
static long curlx_tvdiff(const struct timeval newer, const struct timeval older)
{
  return (newer.tv_sec-older.tv_sec)*1000 +(newer.tv_usec-older.tv_usec)/1000;
}

// Call to reset the stop watch
DBGAPI void ResetTime()
{
	gettimeofday(&start_tv, 0);
	old_tv.tv_sec = start_tv.tv_sec;
	old_tv.tv_usec = start_tv.tv_usec;
}

// Call to print a timing snapshot
DBGAPI void PrintTime(const char* szText)
{
	gettimeofday(&tv, 0);
	DbgOut1(PRINT_LEVEL, LOG_TAG, "%20s: %3i.%03i: %4ims : %3i ms\n",
		szText, tv.tv_sec-start_tv.tv_sec, tv.tv_usec/1000, curlx_tvdiff(tv, start_tv), curlx_tvdiff(tv, old_tv));
	old_tv.tv_sec = tv.tv_sec;
	old_tv.tv_usec = tv.tv_usec;
}

// just for reference
//	struct v4l2_buffer
//    {
//	__u32			index;
//	enum v4l2_buf_type      type;
//	__u32			bytesused;
//	__u32			flags;
//	enum v4l2_field		field;
//	struct timeval		timestamp;
//	struct v4l2_timecode	timecode;
//	__u32			sequence;
//
//	/* memory location */
//	enum v4l2_memory        memory;
//	union {
//		__u32           offset;
//		unsigned long   userptr;
//	} m;
//	__u32			length;
//	__u32			input;
//	__u32			reserved;
//    };

#include <linux/videodev2.h>

DBGAPI void ShowV4L2Buffer(const char *pText, const v4l2_buffer *pBuffer)
{
	pthread_mutex_lock(&LogProtectLevel1);
	PrintMicroSec(PRINT_LEVEL, LOG_TAG);
	if (pBuffer!=NULL)
	{
		DbgOut1(PRINT_LEVEL, LOG_TAG, "%s sequence=%d, index=%d, memtype=%i, userptr=%x, stamp=%i:%i, frames=%i, userbits=%02X%02X%02X%02X\n",
			pText,
			pBuffer->sequence, pBuffer->index,
			pBuffer->memory,
			pBuffer->m.userptr,
			pBuffer->timestamp.tv_sec,
			pBuffer->timestamp.tv_usec/1000,
			pBuffer->timecode.frames,
			pBuffer->timecode.userbits[0],
			pBuffer->timecode.userbits[1],
			pBuffer->timecode.userbits[2],
			pBuffer->timecode.userbits[3]
			);
	}
	else
	{
		DbgOut(PRINT_LEVEL, LOG_TAG, "pBuffer == NULL!");
	}
	pthread_mutex_unlock(&LogProtectLevel1);
}

/*
 * Note these are 7 bit I2C device addresses.
 * The R/W bit is not included and the actual address is in D6..D0.  D7 = 0.
 */
#define PSOC_I2C_ADDR	 		0x40			//!< System control in engine
#define MT9V022_I2C_ADDR 	0x48			//!< Image sensor IT5000 (gen5)
#define EV76C454_I2C_ADDR 	0x18			//!< Image sensor N5600 (gen6)
#define IDT6P50016_I2C_ADDR	0x69			//!< spread spectrum clock chip

const char *GetDeviceName(unsigned char i2c_addr)
{
	const char	*szName="UNKNOWN ";
	if(i2c_addr == PSOC_I2C_ADDR)
		szName="PSOC ";
	else if (i2c_addr == EV76C454_I2C_ADDR)
		szName="JADE ";
	else if (i2c_addr == MT9V022_I2C_ADDR)
		szName="MT9V022 ";
	else if (i2c_addr == IDT6P50016_I2C_ADDR)
		szName="CLOCK ";

	return szName;
}

DBGAPI void ShowI2C(bool write, unsigned char i2c_addr, unsigned char reg, const unsigned char *buf, unsigned char len)
{
	pthread_mutex_lock(&LogProtectLevel1);
	PrintMicroSec(PRINT_LEVEL, "I2C");
	LOG_PRINT(PRINT_LEVEL, "I2C", "I2C %s %s0x%0x, reg=%02x (%02i) ", write ? "Write to" : "Read from", GetDeviceName(i2c_addr), i2c_addr, reg, reg);
	LOG_PRINT(PRINT_LEVEL, "I2C", "len = %i%s", len, len>4 ? "\n" : ": ");
	for (int i=0; i<len; i++)
	{
		LOG_PRINT(PRINT_LEVEL, "I2C", "%02x ", (unsigned int) *buf++);
	}
	LOG_PRINT(PRINT_LEVEL, "I2C", "\n");
	pthread_mutex_unlock(&LogProtectLevel1);
}

