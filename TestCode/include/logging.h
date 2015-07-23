//! \file

#pragma once

#ifndef ANDROID
#define USING_PRINTF 1
#endif

#ifdef USING_PRINTF
#define LOG_OPEN()
#define LOG_CLOSE()

#define PRINT_LEVEL 1

#define LOGE(format, arg...) printf(format, ## arg); puts("")
#define VLOGE(format, ap) vprintf(format, ap); puts("")

#define LOGI(format, arg...) printf(format, ## arg); puts("")

#ifdef ENABLE_DBG_MSG
#define LOGD(format, arg...) printf(format, ## arg); puts("")
#define LOG(format, arg...) printf(format, ## arg); puts("")
#else
#define LOGD(...) ((void)0)
#define LOG(...) ((void)0)
#endif

#define ANDROID_LOG_INFO 0	// just dummy

#define LOGMSG(prio , classname, function) printf("%s-%s", classname, function)

#else // USING_PRINTF
#ifdef ANDROID
#include <android/log.h>

#define PRINT_LEVEL ANDROID_LOG_DEBUG

#define LOG_OPEN()
#define LOG_CLOSE()

#define drv_printk(level, dev, format, arg...)	\
	if (level >= PRINT_LEVEL) __android_log_print(level, dev, format, ## arg)

#define LOGE(format, arg...) \
	drv_printk(ANDROID_LOG_ERROR, LOG_TAG, format, ## arg)
#define VLOGE(format, ap) \
	__android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, format, ap)

#define LOGI(format, arg...) \
	drv_printk(ANDROID_LOG_INFO, LOG_TAG, format, ## arg)

#ifdef ENABLE_DBG_MSG
#define LOGD(format, arg...) \
	drv_printk(ANDROID_LOG_DEBUG, LOG_TAG, format, ## arg)
#define LOG(format, arg...) \
	drv_printk(ANDROID_LOG_DEBUG, LOG_TAG, format, ## arg)
#else
#define LOGD(...) ((void)0)
#define LOG(...) ((void)0)
#endif

//#define LOGMSG(x)  __android_log_print x

#else	                                                                // pure Linux
#include <syslog.h>
//FIXME: needs adjustments
#define LOG_OPEN() \
openlog( LOG_TAG, LOG_CONS|LOG_NDELAY, LOG_USER)
#define LOG_CLOSE() \
closelog()

//FIXME: VLOGE is missing
#define LOGE(format, arg...) syslog( LOG_ERR, format, ## arg)

#define LOGI(format, arg...) syslog( LOG_INFO, format, ## arg)

#ifdef ENABLE_DBG_MSG
#define LOGD(format, arg...) syslog( LOG_DEBUG, format, ## arg)
#define LOG(format, arg...) syslog( LOG_DEBUG, format, ## arg)
#else
#define LOGD(...) ((void)0)
#define LOG(...) ((void)0)
#endif
#endif

//#define ANDROID_LOG_INFO LOG_INFO
//#define LOGMSG(prio , classname, function) \
//syslog( prio, "%s-%s", classname, function)

#endif // USING_PRINTF
