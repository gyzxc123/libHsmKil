//! \file

#if !defined(NO_DEBUG) && !defined(CAMERA_DEVICE_DEBUG_ENABLED)
#warning "CAMERA_DEVICE_DEBUG_ENABLED NOT DEFINED, therefore no camera_device debug"
//#define NO_DEBUG	1
#endif

#include <ctype.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include <utils/Log.h>


#include "CameraScanner.h"

#include "drv/sensor_hal.h"


#define buffer_handle_t	android_buffer_handle_t
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#undef buffer_handle_t

#include "camera_device.h"
#include "queue.h"

#include <android/log.h>

#define LOG_TAG "CamDev"


#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)



using namespace android;
//using namespace std;

#define CAMERA_FOR_SCANNER




class ScannerListener : public CameraListener
{
public:
	virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2) {};
	virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
	virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {};
};

struct acam_buffer_t
{
	void *start;		//!< Points to buffer with image
};

struct hsm_engine_properties	ImagerProps = { HSM_PROPERTIES_V1 };

// Local Variables:
#ifdef CAMERA_FOR_SCANNER
static sp<Camera>	pCamera = NULL;		//!< Camera Object
int fd = -1;

int aim_on_always = 0;
#if 1

#include <fcntl.h>

#include <sys/ioctl.h>

#define CMD_FLAG  's'


#define SE955_CHANNEL_NAME "/dev/se955" 
int fd_scanner1 = -1;



#define AIM_ON		_IOR(CMD_FLAG, 0x00000003, __u32)
#define AIM_OFF	_IOR(CMD_FLAG, 0x00000004, __u32)

void AimOn(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER

	ioctl(fd_scanner1, AIM_ON);
		return ;
	#endif
	
}

void  AimOff(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	ioctl(fd_scanner1, AIM_OFF);
/*
	if(aimoff_enable == 1)
	{
		ioctl(fd_scanner, AIM_OFF);
		aimoff_enable = 0;
	}*/
		return ;
	#endif
	
}
#endif


#else
static sp<CameraScanner> pCamera = NULL;        //!< Camera Object
#endif
static ScannerListener  scannerListener;        //!< Camera callbacks
static acam_buffer_t	*pBuffers = NULL;	//!< Keep track of image buffers
static int		nNumBuffers = 0;	//!< Amount of image buffers we can use
static size_t		cbBufLength = 0;	//!< Buffer length
static queue		bufferQueue;		//!< Buffer queue
static pthread_mutex_t	accessQueue;		//!< Protect queue

#ifdef VIDEO_SUBDEV
static int		video_subdev_handle = INVALID_HANDLE;
#endif

extern void camera_dequeue_buffer_notify(int index, void *pdata);

///////////////////////////////////////////////////////////////////////////////
//! Allocate buffers
/*! 
 \param num_buffers
 \return number of buffers allocated on success, otherwise error code
*/
int camera_allocate_buffers(int num_buffers)
{
	LOGE("%s",__FUNCTION__);

	if( !pBuffers )
	{
		pBuffers = (acam_buffer_t *) malloc(num_buffers * sizeof(pBuffers[0])); 
		if( !pBuffers )
		{
			LOGE("Out of memory");
			return -ENOMEM;
		}

		nNumBuffers = num_buffers;
		memset(pBuffers, 0, num_buffers * sizeof(pBuffers[0]));
	}

	return nNumBuffers;
}

///////////////////////////////////////////////////////////////////////////////
//! Query buffer
/*! 
 \param index
 \return buffer pointer on success, otherwise 0
*/
void* camera_query_buffer(int index)
{
	LOGE("%s",__FUNCTION__);

	/* Checks */
	if( !pBuffers || (index < 0) || (index>=nNumBuffers) || !cbBufLength )
	{
		LOGE("Query buffer error (%d) %s\n", EINVAL, strerror (EINVAL));
		return NULL;
	}

	if( !pBuffers[index].start )
	{
		pBuffers[index].start = malloc(cbBufLength);
		if( !pBuffers[index].start )
		{
			LOGE("Out of memory");
		}
	}

	return pBuffers[index].start;
}

///////////////////////////////////////////////////////////////////////////////
//! Free buffer
/*! 
 \param index
 \param data_ptr
 \return o on success, otherwise error code
*/
int camera_discard_buffer(int index, void *data_ptr)
{

	LOGE("%s",__FUNCTION__);

	/* Checks */
	if( (index < 0) || (index>=nNumBuffers) )
	{
		LOGE("Discard buffer error (%d) %s\n", EINVAL, strerror (EINVAL));
		return NULL;
	}

	if( pBuffers[index].start)
	{
		free(pBuffers[index].start);
		pBuffers[index].start = NULL;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Open the handle to the camera device
/*! 
 \return 0 on success, otherwise error code
*/
int camera_open(void)
{
	LOGE("%s",__FUNCTION__);

	CameraParameters	params;
	status_t		rtn;

	/* Open video device */
	#ifdef CAMERA_FOR_SCANNER
	//pCamera = CameraScanner::connect(0);
	

	
#ifdef CAMERA_FOR_SCANNER
	fd = open("/sys/devices/platform/image_sensor/currDrvIndex",O_RDWR);
    if(fd<0)
	{
		LOGE("Cannot open currDrvIndex\n");
		return -1;
	}
	int temp = 20002;
	write(fd,"20002",5);

	fd_scanner1 = open(SE955_CHANNEL_NAME, O_RDONLY);
 
#endif
	pCamera = Camera::connect(1);
	#else
	pCamera = CameraScanner::connect(ANDROID_CAMERA_ID);
	//pCamera = CameraScanner::connect(0);
	#endif
	if( pCamera == 0)
	{
		LOGE("Cannot connect to camera %d of: %i, %s\n", ANDROID_CAMERA_ID, errno, strerror(errno));
		return -errno;
	}

	LOGE("Camera %d \n", ANDROID_CAMERA_ID);
#ifdef CAMERA_FOR_SCANNER
pCamera->setListener(&scannerListener);	//enable data callback

#else

	pCamera->setScannerListener(&scannerListener);	//enable data callback
#endif

#ifdef VIDEO_SUBDEV 
	/* Open subdevice */
	video_subdev_handle = ::open(VIDEO_SUBDEV, O_RDWR);
	if (video_subdev_handle == -1) {
		LOGE("ERROR opening %s: %d\n", VIDEO_SUBDEV, errno);
		return -errno;
	}

	LOGE("Subdevice: %s, handle = %d\n", VIDEO_SUBDEV, video_subdev_handle);
#endif

	ImagerProps.width = 256; ImagerProps.height = 64;
#ifdef CAMERA_FOR_SCANNER
			//ImagerProps.width = 624;
			//ImagerProps.height = 474;
			ImagerProps.width = 640;
			ImagerProps.height = 480;
			ImagerProps.mount = 2;
			ImagerProps.i2c_addr_sensor = 0x48;
			ImagerProps.i2c_addr_psoc = 0x40;
			ImagerProps.i2c_addr_clock = 0x69;
#else

	if( 0 != camera_ioctl(HSM_GET_PROPERTIES, &ImagerProps) )
	{
		pCamera->disconnect();
		LOGE("HSM_GET_PROPERTIES error %d\n", errno);
		return -1;
	}
#endif

#ifdef CAMERA_FOR_SCANNER
#else

	LOGE("Image size = %dx%d\n", ImagerProps.width, ImagerProps.height);
#endif

	cbBufLength = ImagerProps.height * ImagerProps.width;

	params.unflatten(pCamera->getParameters());
	params.set("mtk-cam-mode", 1);
#if 0 //def CAMERA_FOR_SCANNER
#else
	params.set("scanner-mode", "on");
#endif
	params.setPreviewSize(ImagerProps.width, ImagerProps.height);
	//params.setPreviewFormat("yuv422i-yuyv"); // FIXME: "raw"
	//params.setPreviewFormat("yuv420sp"); // FIXME: "raw" 
	rtn = pCamera->setParameters(params.flatten());
	if( rtn != OK )
	{
		LOGE("setParameters error %d\n", rtn);
	}
#ifdef CAMERA_FOR_SCANNER


	if(fd>=0)write(fd,"20001",5);
#endif

#if 0
	rtn = pCamera->setPreviewTexture(dummy_texture); // FIXME: Is there a dummy texture?
	if( rtn != OK )
	{
		KIL_ERR("setPreviewDisplay error %d\n", rtn);
	}
#endif

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Control the camera device
/*! 
 \param request
 \param parg
 \return 0 on success, otherwise error code
*/
int camera_ioctl(int request, void *parg)
{
	LOGE("%s",__FUNCTION__);

	int	ret = 0;

#ifdef CAMERA_FOR_SCANNER
return 0;

if(request == HSM_GPIO_WRITE)
{
hsm_gpio_data *pScannerGpio = (hsm_gpio_data *)parg;

LOGE("ZZZ HSM_GPIO_WRITE pin:%d,val:%d\n",pScannerGpio->pin,pScannerGpio->value);
}
else
{
	return 0;

}

#endif

#ifdef VIDEO_SUBDEV
	ASSERT(video_subdev_handle>=0);
	ret = ioctl(video_subdev_handle, request, parg);
#else
	status_t	rtn;
	int32_t		cb;

	cb = _IOC_SIZE(request);
#if 1
	rtn = pCamera->sendCommand(288, request, (int32_t)parg);
#else
	rtn = pCamera->sendCommand(request, (int32_t)parg, (int32_t)&cb);
#endif
	if( rtn != OK )	
	{
		LOGE("sendCommand failed, error = %d\n", rtn);
		ret = (rtn < 0)? rtn : -1;
	}
#endif
	return ret;
}


///////////////////////////////////////////////////////////////////////////////
//! Initialize the camera interface
/*! 
 \param capture_width
 \param capture_height
 \return 0 on success, otherwise error code
*/
int camera_init(void)
{
	LOGE("%s",__FUNCTION__);

	int result;

	/* Open video device */
	LOGE("CALL camera_open...\n");
	result = camera_open();
	if( result < 0 )
	{
		LOGE("failed to open camera, error=%d\n", -result);
		return result;
	}

	/* Create buffer queue */
	LOGE("Create buffer queue...\n");
	bufferQueue = createQueue();
	pthread_mutex_init(&accessQueue, NULL);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Dequeue the buffer
/*! 
 \return buffer on success, otherwise error code
 Blocks if no buffer is completed
*/
static int camera_dequeue_buffer(void)
{
	LOGE("%s",__FUNCTION__);

	int	index;

	/* Pop the buffer off the buffer queue */
	pthread_mutex_lock(&accessQueue);
	index = bufferQueue.size? bufferQueue.pop(&bufferQueue) : -1;
	pthread_mutex_unlock(&accessQueue);

	LOGE("%s: Dequeue index = %d\n", __FUNCTION__, index);

	/* Return */
	return index;
}

///////////////////////////////////////////////////////////////////////////////
//! Enqueue the buffer
/*! 
 \param index
 \return 0 on success, otherwise error code
*/
int camera_queue_buffer(int index)
{
	LOGE("%s",__FUNCTION__);

	/* Checks */
	if( (index < 0) || (index>=nNumBuffers) )
	{
		LOGE("Queue buffer error (%d) %s\n", EINVAL, strerror (EINVAL));
		return NULL;
	}

	/* Push the buffer onto the buffer queue */
	pthread_mutex_lock(&accessQueue);
	bufferQueue.push(&bufferQueue, index);
	pthread_mutex_unlock(&accessQueue);

	/* Return */
	return 0;
}



void ScannerListener::postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
	LOGE("%s",__FUNCTION__);

	static int postdata_count = 0;

	if( msgType & CAMERA_MSG_PREVIEW_FRAME )
	{
		int	index;
#if 1
		postdata_count++;

		if(postdata_count >1 || aim_on_always == 1)
		{
			
			AimOn();
			postdata_count = 0;
		}
#endif
		
		

		index = camera_dequeue_buffer();
		if( (index >= 0) && pBuffers[index].start )
		{
			void	*ptr;

			ptr = dataPtr->pointer();
			if( ptr )
			{
				//LOGE("postData index %d start,%s",index,__FUNCTION__);
				memcpy(pBuffers[index].start, ptr, cbBufLength);
				//LOGE("postData index %d end,%s",index,__FUNCTION__);
				camera_dequeue_buffer_notify(index, 0);
			}
			else
			{
				LOGE("%s: Failed to get dataPtr", __FUNCTION__);
			}
		}
#if 1
		if(aim_on_always == 0)
		{
			AimOff();
		}
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Start streaming
/*! 
 \return 0 on success, otherwise error code
*/
int camera_streamon()
{
	status_t	rtn;

	LOGE("%s",__FUNCTION__);

	aim_on_always = 0;

	/* Stream on */
	if( !pCamera->previewEnabled() )
	{
	#ifdef CAMERA_FOR_SCANNER
	pCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);
	#else
		pCamera->setPreviewCallbackFlag(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);
	#endif
		rtn = pCamera->startPreview();
		if( rtn != OK )
		{
			LOGE("startPreview error (%d)\n", rtn);
			return -1;
		}
	}

	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//! Stop streaming. Removes all queued buffers
/*! 
 \return 0 on success, otherwise error code 
*/
int camera_streamoff()
{
	LOGE("%s",__FUNCTION__);

	aim_on_always = 1;

	//pCamera->stopPreview();//temp temp temp temp

	/* Clear queue */
	pthread_mutex_lock(&accessQueue);
	while( bufferQueue.size )
	{
		bufferQueue.pop(&bufferQueue);
	}
	pthread_mutex_unlock(&accessQueue);

	return 0;
}


int camera_stoppreview()
{
	LOGE("%s",__FUNCTION__);

	pCamera->stopPreview();

	/* Clear queue */
	#if 0
	pthread_mutex_lock(&accessQueue);
	while( bufferQueue.size )
	{
		bufferQueue.pop(&bufferQueue);
	}
	pthread_mutex_unlock(&accessQueue);
	#endif

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//! Deinitialize the camera interface
/*! 
*/
void camera_deinit(void)
{
	LOGE("%s into",__FUNCTION__);

#ifdef VIDEO_SUBDEV
	if( video_subdev_handle != INVALID_HANDLE )
	{
		::close (video_subdev_handle);
		video_subdev_handle = INVALID_HANDLE;
	}
#endif
	/* Release camera */
	if( pCamera != 0 )
	{
		camera_streamoff();
		LOGE("Disconnect Camera");
		pCamera->disconnect();
		pCamera = NULL;
	}

	if( pBuffers )
	{
		free(pBuffers);
		pBuffers = NULL;
		nNumBuffers = 0;
	}

	pthread_mutex_destroy(&accessQueue);
#ifdef CAMERA_FOR_SCANNER
	int temp = 20001;
	if(fd>=0)write(fd,"20001",5);
	close(fd);
	close(fd_scanner1);
#endif

	LOGE("%s exit",__FUNCTION__);
}

