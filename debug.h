#if !defined(NO_DEBUG) && !defined(LOG_TAG)
#warning "LOG_TAG is not defined"
#define LOG_TAG	""
#endif

#include <DebugHelpers.h>

//! DEBUG DEFINES: (TODO: move me to a debug header??? - found it easier to define here)
//#define CAMERA_DEVICE_DEBUG_ENABLED	// used to enable camera_device debug
//#define CAPTURE_DEBUG_ENABLED		// used to enable capture debug
//#define HWLAYER_SKEL_DEBUG_ENABLED	// used to enable hwlayer_skel debug
//#define I2C_DEBUG_ENABLED		// used to enable i2c debug
//#define GPIO_DEBUG_ENABLED		// used to enable gpio debug
//#define QUEUE_BUFFER_DEBUG_ENABLED	// used to enable queue_buffer debug

// TODO: always enabled???
#define KIL_ERR(format, arg...) 	DBG_ERR1(format, arg)	//printf(format, ## arg)

