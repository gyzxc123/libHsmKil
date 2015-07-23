#define LOG_TAG "Capture"
#if !defined(NO_DEBUG) && !defined(CAPTURE_DEBUG_ENABLED)
#warning "CAPTURE_DEBUG_ENABLED NOT DEFINED, therefore no capture debug"
#define NO_DEBUG	1
#endif

#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#include "queue.h"
#include "camera_device.h"
#include "capture.h"


#ifndef DBG_STAMP_IMAGE
#define DBG_STAMP_IMAGE		0
#endif

#if (NUM_BUFFERS<6)
//#error We need at least 5 buffers
#endif

#ifndef NUM_REQUIRED_BUFFERS
#define NUM_REQUIRED_BUFFERS    3	// Total number of REQUIRED 'system' buffers (for IMX6 only)
#endif

#if NUM_REQUIRED_BUFFERS > NUM_BUFFERS
#error " NUM_REQUIRED_BUFFERS > NUM_BUFFERS"
#endif

#define IMG_CHECK_VAL 0x00FE00FE

// Image Buffer Structure (internal to KIL)
struct ImageBuffer_t
{
	void *start;		//!< Points to buffer with image
	size_t length;		//!< Size of buffer
	bool InApp;		//!< Help with buffer debugging
	int handle;		//!< Handle to the scan driver
	bool IsQueued;		//!< Is this buffer queued?
	bool IsReleased;	//!< Is this buffer released by Scan Driver?
	unsigned char *pStampCheckLocation;	//!< Stamp check location
};	

// Local Variables:
static ImageBuffer_t	*pBuffers;		//!< Keep track of image buffers
static int		nNumBuffers = 0;	//!< Amount of image buffers we can use
static int		nAutoBuffers;		//!< Amount of image buffers to queue automatically
static int		nCurAutoBuffers = 0;	//!< Enables/Disables auto queueing
static queue		bufferQueue;		//!< Requested buffer queue
static pthread_mutex_t	accessQueue;		//!< Protect queue
static bool		bIsReleased;		//!< IsReleased flag 
static unsigned int	cBufQueue = 0;		//!< Count of queued buffers 
static unsigned int	cBufReqIn = 0;		//!< Count of requested buffers
static unsigned int	cBufReqOut = 0;		//!< Count of queued requested buffers
static vsync_notify_t	pfVsyncNotify;		//!< Vsync callback
static void		*pDataVsync;		//!< Vsync data

// Local Prototypes:

#ifndef NO_DEBUG
inline char dbg_get_buffer_state(int index)
{
	return pBuffers[index].IsQueued? (pBuffers[index].IsReleased? 'Q' : 'R') : (pBuffers[index].IsReleased? 'F' : 'B');
}

static void DBG_BUFFERS_STATE(const char *title)
{
	int	index;
	char	str[256];

	for( index = 0; index < nNumBuffers; ++index )
	{
		str[index] = dbg_get_buffer_state(index);

		/*DBG_OUT1("%s: Buffer %d is %s %s\n", __FUNCTION__, index,
			pBuffers[index].IsQueued? "Queued" : "Loose",
			pBuffers[index].IsReleased? "Free" : "Busy");*/
	}

	str[index] = '\0';
	DBG_OUT1("%s: Buffers state: %s\n", title, str);
}
#else
#define DBG_BUFFERS_STATE(FUN)	((void) 0)
#endif

#if DBG_STAMP_IMAGE
///////////////////////////////////////////////////////////////////////////////
//! Stamps the image with a pattern
/*! 
 \param index
*/
static void capture_stamp_image(int index)
{
	/* Load check values */
	*(unsigned long *)pBuffers[index].pStampCheckLocation = IMG_CHECK_VAL;
	DBG_OUT1("%s: load check values 0x%X\n", __FUNCTION__, *(unsigned long *)pBuffers[index].pStampCheckLocation);
}

///////////////////////////////////////////////////////////////////////////////
//! Checks for the stamp in the image
/*! 
 \param index
 \return 0 on success, otherwise false
*/
static bool capture_check_stamp(int index)
{
	/* Check and return */
	return *(unsigned long *)pBuffers[index].pStampCheckLocation != IMG_CHECK_VAL;
}
#endif	// DBG_STAMP_IMAGE


///////////////////////////////////////////////////////////////////////////////
//! Register vsync notification with the scan driver (callback)
/*! 
 \param pfNotification 
 \param data  
 \return 1 on success 
*/
unsigned long capture_register_vsync_notification(vsync_notify_t pfNotification, void * data)
{
	DBG_FUNC();
	pfVsyncNotify = pfNotification;
	pDataVsync = data;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//! Get the pointer to the buffer based on the index
/*! 
 \param index
 \return pointer to buffer
*/
void *capture_get_buffer_pointer(int index)
{
	if((index >=0) && (index < NUM_BUFFERS))
	{
		return pBuffers[index].start;
	}
	else
	{
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Set the handle to the buffer
/*! 
 \param index
 \param handle
 \return 0 on success, otherwise error code
*/
int capture_set_buffer_handle(int index, int handle)
{
	if((index >=0) && (index < NUM_BUFFERS))
	{
		pBuffers[index].handle = handle;
		return 0;
	}
	else
	{
		return -EINVAL;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the image buffers (KIL buffers)
/*! 
 \return 0 on success, otherwise error code
*/
static int capture_init_image_buffers()
{
	DBG_FUNC();

	int	index;

	nNumBuffers = camera_allocate_buffers(NUM_BUFFERS);
	if( nNumBuffers < 0 )
	{
		KIL_ERR("Fails to allocate buffers (%d) %s", nNumBuffers, strerror(nNumBuffers));
		return nNumBuffers;
	}

	DBG_OUT1("Buffers = %i\n", nNumBuffers);

	pBuffers = (struct ImageBuffer_t *) calloc(nNumBuffers, sizeof (*pBuffers)); // fixme: move to ctor
	if (!pBuffers)
	{
		DBG_OUT1("Out of memory");
		return (ENOMEM);	// fixme: cleanup of VIDIOC_REQBUFS required?
	}

	for( index = 0; index < nNumBuffers; ++index )
	{
		DBG_OUT1("Init pBuffers[%d]\n", index);

		pBuffers[index].InApp = false;
		pBuffers[index].length = ImagerProps.width * ImagerProps.height;
		pBuffers[index].start = camera_query_buffer(index);
		pBuffers[index].handle = index + 1; // off by 1
		pBuffers[index].IsQueued = false;
		pBuffers[index].IsReleased = true;
		pBuffers[index].pStampCheckLocation = ((unsigned char*)pBuffers[index].start + pBuffers[index].length - 4);

		if( !pBuffers[index].start )
		{
			KIL_ERR("Failed to query buffer %i of %i\n", index, nNumBuffers);
			return -errno;
		}

		DBG_OUT1("Buffer[%i] = %X, len=%i\n", index, pBuffers[index].start, pBuffers[index].length);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the capture interface
/*! 
 \return capture mode on success, othewise error code 
*/
int capture_init(int capture_mode)
{
	DBG_FUNC();

	int result;

	/* Initialize image buffers */
	DBG_OUT1("Call camera_init_image_buffers...\n");
	result = capture_init_image_buffers();
	if(result < 0)
	{
		KIL_ERR("failed to open camera, error=%d\n", result);
		return result;
	}

	/* Create buffer queue */
	DBG_OUT1("Create buffer queue...\n");
	bufferQueue = createQueue();
	pthread_mutex_init(&accessQueue, NULL);

	switch( capture_mode )
	{
	case 4:
		nAutoBuffers = nNumBuffers;	// Queue all buffers
		bIsReleased = false;
		break;

	default:
		capture_mode = 6;
	case 6:
	case 5:
	case 7:
		nAutoBuffers = NUM_REQUIRED_BUFFERS;	//TODO: Request from camera device
		bIsReleased = true;
		break;
	}
	
	return capture_mode;
}

///////////////////////////////////////////////////////////////////////////////
//! Enqueue the buffer
/*! 
 \param min_required_buffers
 \return 0 on success, otherwise error code
*/
static int capture_queue_buffer(int index)
{
	int	ret;

#if DBG_STAMP_IMAGE
	/* Stamp the image */
	DBG_OUT1("Stamp image b4 QBUF\n");
	capture_stamp_image(index);
#endif

	ret = camera_queue_buffer(index);
	if( ret >= 0 )
	{
		++cBufQueue;
		/* Indicate that this buffer is queued. */
		pBuffers[index].IsQueued = true;
		pBuffers[index].IsReleased &= bIsReleased;

		DBG_OUT1("Queued index = %d-%c\n", index, dbg_get_buffer_state(index));
	}
	else
	{
		//TODO: handle the error
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//! Enqueue the buffers
/*! 
 \return number of queued buffers on success, otherwise error code
*/
static int capture_queue_buffers(void)
{
	int	index;
	int	count = 0;


	DBG_BUFFERS_STATE(__FUNCTION__);

	/* Iterate through all buffers */
	for( index = 0; index < nNumBuffers; ++index )
	{
		pthread_mutex_lock(&accessQueue);

		if( !cBufReqIn && !(cBufQueue < nCurAutoBuffers) )
		{
			pthread_mutex_unlock(&accessQueue);
			break;
		}

		/* Is this buffer not queued && released by SD, then queue */
		if( !pBuffers[index].IsQueued && pBuffers[index].IsReleased )
		{
			if( capture_queue_buffer(index) >= 0 )
			{
				if( cBufReqIn ) --cBufReqIn;
				++count;
			}
		}

		pthread_mutex_unlock(&accessQueue);
	}

	return count;
}

///////////////////////////////////////////////////////////////////////////////
//! notification callback - dequeue buffer and notify Scan Driver
/*! 
 \param index  
 \return 1 on success 
*/
void camera_dequeue_buffer_notify(int index, void *pdata)
{
	DBG_OUT1("~~~~~~~~~~~ Received buffer!! %d-%c\n", index, dbg_get_buffer_state(index));

#if DBG_STAMP_IMAGE
	/* Check stamp */
	if( !capture_check_stamp(index) )
		DBG_OUT1("!!!!! Check image FAILED !!!!!\n");
	else
		DBG_OUT1("+++++ Check image PASSED +++++\n");
#endif

	/* Dequeue buffer */
	pthread_mutex_lock(&accessQueue);

	if( cBufReqOut && pBuffers[index].IsReleased )
	{
		--cBufReqOut;
		pBuffers[index].IsReleased = false;
	}

	if( cBufQueue ) --cBufQueue;
	/* Set flag to indicate the buffer is no long queued */
	pBuffers[index].IsQueued = false;
	pBuffers[index].InApp = true;

	pthread_mutex_unlock(&accessQueue);

	/* Debug store image */
	//DBG_STORE_IMAGE((unsigned char *) capture_get_buffer_pointer(index), DBG_STAMP_HWL);

	if( !pBuffers[index].IsReleased )
	{
		/* Notify Vsync */
		DBG_OUT1("BEFORE Callback, vsync_cb_data=0x%x\n", pDataVsync);
		pfVsyncNotify(pBuffers[index].handle, pDataVsync);
		DBG_OUT1("AFTER Callback\n");
	}

	/* it's ok to queue */	
	capture_queue_buffers();
}

///////////////////////////////////////////////////////////////////////////////
//! Called from the Scan Driver to request a buffer (control of the buffer passed to the SD)
//! in capture modes 1, 4 , 6
/*! 
 \param index 
 \return 0 if success, otherwise error code
*/
int capture_request_buffer(void)
{
	DBG_FUNC();

	int	rtn;

	DBG_OUT1("%s: buffer requested from SD\n", __FUNCTION__);
	pthread_mutex_lock(&accessQueue);

	++cBufReqIn;
	++cBufReqOut;

	pthread_mutex_unlock(&accessQueue);

	/* it's okay to queue */
	rtn = capture_queue_buffers();

	return rtn? 0 : -EAGAIN;
}

///////////////////////////////////////////////////////////////////////////////
//! Called from the Scan Driver to request a specific buffer (control of the buffer passed to the SD)
//! in capture modes 2, 3 , 5, 7
/*! 
 \param index 
 \return 0 if success, otherwise error code
*/
int capture_request_buffer(int index)
{
	DBG_FUNC();

	int	rtn;

	DBG_OUT1("%s: index = %d requested from SD\n", __FUNCTION__, index);
	ASSERT(index>=0);		// Valid index
	ASSERT(index<nNumBuffers);	// valid index

	pthread_mutex_lock(&accessQueue);

	if( pBuffers[index].IsQueued || ((rtn = capture_queue_buffer(index)) >= 0) )
	{
		pBuffers[index].IsReleased = false;
		rtn = 0;
	}

	pthread_mutex_unlock(&accessQueue);

	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//! Called from the Scan Driver to release a specific buffer (control of the buffer passed back to the KIL)
/*! 
 \param index 
*/
void capture_release_buffer(int index)
{
	/* Release this buffer  */
	DBG_OUT1("%s: index = %d released from SD\n", __FUNCTION__, index);
	ASSERT(index>=0);		// Valid index
	ASSERT(index<nNumBuffers);	// Valid index

	pthread_mutex_lock(&accessQueue);
	pBuffers[index].IsReleased = true;
	pthread_mutex_unlock(&accessQueue);

	/* it's okay to queue */
	capture_queue_buffers();
}

///////////////////////////////////////////////////////////////////////////////
//! Start scanning
/*! 
 \return true on succes, otherwise false 
*/
bool capture_start_scanning()
{
	DBG_FUNC();

	int	rtn;
	
	pthread_mutex_lock(&accessQueue);
	nCurAutoBuffers = nAutoBuffers;		// Enables auto queueing
	pthread_mutex_unlock(&accessQueue);

	capture_queue_buffers();

	rtn = camera_streamon();

	return (rtn == 0);
}

///////////////////////////////////////////////////////////////////////////////
//! Stop scanning
/*! 
 \return true on succes, otherwise false  
*/
bool capture_stop_scanning()
{
	DBG_FUNC();

	int	index;

	pthread_mutex_lock(&accessQueue);

	/* This also removes all queued buffers */
	camera_streamoff();

	nCurAutoBuffers = 0;	// Disable auto queueing

	/* Clear queue */
	cBufQueue = cBufReqOut = 0;

	while( bufferQueue.size )
	{
		bufferQueue.pop(&bufferQueue);
	}

	for( index = 0; index < nNumBuffers; ++index )
	{
		if( pBuffers[index].IsQueued )
		{
			pBuffers[index].IsQueued = false;
			pBuffers[index].IsReleased = true;
		}
	}

	pthread_mutex_unlock(&accessQueue);

	DBG_BUFFERS_STATE(__FUNCTION__);

	return true;
}

bool capture_stop_preview()
{
	DBG_FUNC();
	camera_stoppreview();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//! Deinitialize the capture interface
/*! 
 \return 0 on success 
*/
int capture_deinit()
{
	DBG_FUNC();

	int result;

	capture_stop_scanning();
	pthread_mutex_destroy(&accessQueue);

	/* Discard buffers */
	for(int i = 0; i < nNumBuffers; ++i)
	{
		camera_discard_buffer(i, pBuffers[i].start);
	}

	/* Free image buffers */
	free (pBuffers);
	pBuffers = NULL;

	return 0;
}
