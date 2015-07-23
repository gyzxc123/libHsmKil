
#pragma once

#include <stdarg.h>

#ifndef MSCGEN
#    ifdef UNITTEST
#       define MSCGEN 1 //!< MSC messages 1: Enable, 0: Disable
#    else
#       define MSCGEN 0 //!< MSC messages 1: Enable, 0: Disable
#    endif
#endif

#if MSCGEN

//@{
/** @name POS_ARG
    POS, POS_FWD POS_ARG can be used to pass caller location
	information to a function.  These macros expand to blanks when
	MSCGEN = 0 (diabled).

	@c log_caller, below is a function that records information about
	where it was called from.

	@code
	void log_caller(int str POS_ARG)
    {
	    msc_note("log_caller(%s) called from %u,%s", str, _line, _func_name)
    }
	@endcode

	If MSCGEN is TRUE, POS_ARG will add parameters _line and
	_func_name to @c log_caller.  @c log_caller uses @c msc_note to
	record the caller's location.

	Code that wants to call log_caller can do this
	@code
	log_caller("string" POS);
	@endcode


	@c POS_FWD can forward caller information through an function
	caller.  This is often useful when a wrapper needs to calls a
	funtion that logs caller infromation.

	@code
	void wrap_log_caller(int str POS_ARG);
	{
	    log_caller(str POS_FWD);
	}
	@endif

	This wrap_log_caller will forward it's caller information to
	log_caller.  So, the location where wrap_log_caller will be
	recorded in the log instead of the location where @c
	wrap_log_caller called @c log_caller.
*/

#define POS_ARG , const char * _func_name, unsigned _line
#define POS , __FUNCTION__, __LINE__
#define POS_FWD , _func_name, _line
#else
#define POS_ARG
#define POS
#define POS_FWD
#endif
//@} Endof POS_ARG group.

enum MSCGEN_CONTROL
{
    // If true, shows the state of syn objects like event and critical sections
	MSCGEN_SYNC_STATE       = 1 << 0,
	MSCGEN_VSYNC            = 1 << 1,
	MSCGEN_START_OF_FRAME   = 1 << 2,
	MSCGEN_LINE_DATA        = 1 << 3,
	MSCGEN_IIC              = 1 << 4,
	MSCGEN_BUFFER_QUEUE     = 1 << 5,
	MSCGEN_CRITICAL_SECTION = 1 << 6,
	MSCGEN_APP_INTERFACE    = 1 << 7,  // Logs of called from app code.
	MSCGEN_HWLAYER          = 1 << 8,
	MSCGEN_FRAMING          = 1 << 9, // Logs update cycle, frame
								   // boundary operations
	MSCGEN_EXPOSURE_CONTROL = 1 << 10,
	MSCGEN_LIGHT_CONTROL    = 1 << 11,
	MSCGEN_POWER_MANAGEMENT = 1 << 12,


	MSCGEN_IIC_VERBOSE      = 1 << 17,
	MSCGEN_HWLAYER_VERBOSE  = 1 << 16,
	MSCGEN_VERBOSE          = MSCGEN_HWLAYER_VERBOSE | MSCGEN_IIC_VERBOSE,
};

#ifdef UNITTEST
    enum {
		mscgen_control = MSCGEN_CONTROL( ~0U
//					  & ~MSCGEN_SYNC_STATE
//					  & ~MSCGEN_VSYNC
//					  & ~MSCGEN_START_OF_FRAME
//					  & ~MSCGEN_LINE_DATA
//					  & ~MSCGEN_IIC
//					  & ~MSCGEN_IIC_VERBOSE
//					  & ~MSCGEN_BUFFER_QUEUE
//					  & ~MSCGEN_CRITICAL_SECTION
//					  & ~MSCGEN_APP_INTERFACE
//					  & ~MSCGEN_HWLAYER
//					  & ~MSCGEN_HWLAYER_VERBOSE
//					  & ~MSCGEN_FRAMING
//					  & ~MSCGEN_EXPOSURE_CONTROL
//					  & ~MSCGEN_LIGHT_CONTROL
//					  & ~MSCGEN_POWER_MANAGEMENT
//					  & ~MSCGEN_VERBOSE
		   )
	};
#else
	enum {
		mscgen_control = ( 0
//					  | MSCGEN_SYNC_STATE
						   | MSCGEN_VSYNC
						   | MSCGEN_START_OF_FRAME
//					  | MSCGEN_LINE_DATA
						   | MSCGEN_IIC
					  | MSCGEN_IIC_VERBOSE
//					  | MSCGEN_BUFFER_QUEUE
//						   | MSCGEN_CRITICAL_SECTION
						   | MSCGEN_APP_INTERFACE
						   | MSCGEN_HWLAYER
					  | MSCGEN_HWLAYER_VERBOSE
//					  | MSCGEN_FRAMING
//					  | MSCGEN_EXPOSURE_CONTROL
//					  | MSCGEN_LIGHT_CONTROL
//					  | MSCGEN_POWER_MANAGEMENT
//					  | MSCGEN_VERBOSE
			)
	};
#endif

#define application   "AP"
#define scanmanager   "SM"
#define scanengine    "SE"
#define scancallback  "SC"
#define hwlayer       "HW"
#define eof_thread	  "EO"
#define camera		  "CA"
#define v4l2		  "VL"

void vmscc(  const char * top_level, const char * op, const char * bottom_level,
		   unsigned color, const char * msg, va_list argptr     );


void msc(const char * top_level, const char * op, const char * bottom_level,
		 const char * msg, ...            );

void msc_warning(const char * msg, ...);
void msc_error(const char * msg, ...)	;
void msc_note(const char * msg, ...);

/** An instance of Msc_return will generate an MSC message when an
	object is destroyed.  This is useful for printing a message when a
	function with returns.

	@code
	int my_func()
    {
	   int nReturn = 0;
	   Msc_return<BOOL> ret_help( application, "<<", scanmanager,
	                              "my_func returns %u", nReturn    );

       return nReturn
    }
	@endcode
*/
/* template <class RET> */
/* class Msc_return */
/* { */
/* public: */
/* 	Msc_return(  const char * _top_level, const char * _op,  */
/* 				 const char * _bottom_level,  */
/* 				 const char * _msg, const RET & _ret         ) */
/* 		: top_level(_top_level), */
/* 		  op(_op), */
/* 		  bottom_level(_bottom_level), */
/* 		  msg(_msg), */
/* 		  ret(_ret) */
/* 		{ */
/* 		} */

/* 	~Msc_return() */
/* 		{ */
/* 			msc(top_level, op, bottom_level, msg, ret); */
/* 		} */

/* private: */
/* 	// default and copy construct, and assigment not allowed. */
/* 	Msc_return(); */
/* 	Msc_return(const Msc_return &); */
/* 	void operator =(const Msc_return &); */

/* 	const char * top_level, * op, * bottom_level; */
/* 	const char * msg; */
/* 	const RET & ret; */
/* }; */

#if MSCGEN
#define MSC(control, top_level, op, bottom_level, msg, arg...) \
	if (mscgen_control & control)	{ msc(top_level, op, bottom_level, msg, ## arg); }

#define MSC_WARNING(control, msg, arg...) \
	if (mscgen_control & control)	{ msc_warning(msg, ## arg); }
#define MSC_NOTE(control, msg, arg...) \
	if (mscgen_control & control)	{ msc_note(msg, ## arg); }
#define MSC_ERROR(control, msg, arg...) \
	if (mscgen_control & control)	{ msc_error(msg, ## arg); }
#else
#define MSC(control, top_level, op, bottom_level, msg, ...) ((void)0)
#define MSC_WARNING(control, msg, arg...)  ((void)0)
#define MSC_NOTE(control, msg, arg...)  ((void)0)

#ifdef LOGERRORS
#define MSC_ERROR(control, msg, arg...) \
	if (mscgen_control & control)	{ LOGE(msg, ## arg); }
#else
#define MSC_ERROR(control, msg, arg...) ((void)0)
#endif

#endif // MSCGEN



