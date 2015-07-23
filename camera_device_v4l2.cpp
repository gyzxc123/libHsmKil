//! \file
#define LOG_TAG "CamDev"
#if !defined(NO_DEBUG) && !defined(CAMERA_DEVICE_DEBUG_ENABLED)
#warning "CAMERA_DEVICE_DEBUG_ENABLED NOT DEFINED, therefore no camera_device debug"
#define NO_DEBUG	1
#endif

#include "CommonInclude.h"
#include <ctype.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <linux/videodev2.h>
#include "camera_device.h"

#ifdef MEDIA_DEVICE
#include "mediadev.h"
#endif

using namespace std;

struct hsm_engine_properties	ImagerProps = { HSM_PROPERTIES_V1 };

// Local Variables:
static int		video_device_handle = INVALID_HANDLE;	//!< File descriptor for video device
const v4l2_memory	memtype = V4L2_MEMORY_MMAP; 		//!< Either V4L2_MEMORY_USERPTR or V4L2_MEMORY_MMAP
static size_t		cbBufLength = 0;			//!< Buffer length
static pthread_t	dequeue_thread_id;			//!< Dequeue thread id
static pthread_mutex_t	accessDeque;				//!< Protect Deque condition
static pthread_cond_t	cvDeque;				//!< Wait for Deque
static bool		bStreamOn;				//!< Status of VIDIOC_STREAMON and VIDIOC_STREAMOFF
static unsigned int	cBufQueue = 0;				//!< Count of queued buffers 
static bool		bExitThread;				//!< Flag to exit thread

#ifdef VIDEO_SUBDEV
static int	video_subdev_handle = INVALID_HANDLE;
#define IOCTL_DEVICE_HANDLE	video_subdev_handle
#else
#define IOCTL_DEVICE_HANDLE	video_device_handle
#endif

#define DEQUE_TIMEOUT_MS	1000

extern void camera_dequeue_buffer_notify(int index, void *pdata);

// Local Prototypes:
static int camera_open(void);
static void *dequeue_thread(void* pParm);

inline void set_timeval_ms(timeval *ptimeval, time_t ms)
{
	ptimeval->tv_sec = ms / 1000;
	ptimeval->tv_usec = (ms % 1000) * 1000;
}

inline time_t get_timeval_ms(timeval *ptimeval)
{
	return (ptimeval->tv_sec * 1000) + (ptimeval->tv_usec / 1000);
}

///////////////////////////////////////////////////////////////////////////////
//! Open the handle to the camera device
/*! 
 \param capture_width
 \param capture_height
 \return 0 on success, otherwise error code
*/
int camera_open(void)
{
	DBG_OUT1("CALL camera_open\n");

#ifdef MEDIA_DEVICE
	if( media_setup_links() < 0 )
	{
		KIL_ERR("ERROR setup media links: %i\n", errno);
		return -errno;
	}
#endif

#ifdef VIDEO_SUBDEV
	/* Open subdevice */
	video_subdev_handle = ::open(VIDEO_SUBDEV, O_RDWR);
	if (video_subdev_handle == -1) {
		KIL_ERR("ERROR opening %s: %d\n", VIDEO_SUBDEV, errno);
		return -errno;
	}

	DBG_OUT1("Subdevice: %s, handle = %d\n", VIDEO_SUBDEV, video_subdev_handle);
#endif

	/* Open video device */
	video_device_handle = ::open (VIDEO_DEVICE, O_RDWR, 0);
	if (-1 == video_device_handle)
	{
		DBG_OUT1("Cannot open %s: %i, %s\n", VIDEO_DEVICE, errno, strerror (errno));
		return -errno;
	}

	DBG_OUT1("Video device: %s, handle = %d\n", VIDEO_DEVICE, video_device_handle);

	if( -1 == camera_ioctl(HSM_GET_PROPERTIES, &ImagerProps) )
	{
		DBG_OUT1("HSM_GET_PROPERTIES error %d", errno);
		return -errno;
	}

#ifdef MEDIA_DEVICE
	media_setup_capture_subdev(ImagerProps.width, ImagerProps.height);
#endif

	/* Query capabilities: */

	v4l2_capability Cap;
	if (-1 == ioctl (video_device_handle, VIDIOC_QUERYCAP, &Cap))
	{
		if (EINVAL == errno)
		{
			DBG_OUT1("%s  is no V4L2 device\n", VIDEO_DEVICE);
			return (-errno);
		} else {
			DBG_OUT1("VIDIOC_QUERYCAP error (%d) %s\n", errno, strerror (errno));
			return (-errno);
		}
	}
	if (!(Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		DBG_OUT1("%s is no video capture device\n", VIDEO_DEVICE);
		return (-EINVAL);
	}
	if (!(Cap.capabilities & V4L2_CAP_STREAMING))
	{
		DBG_OUT1("%s does not support streaming i/o\n", VIDEO_DEVICE);
		return (-EINVAL);
	}

	/* Crop capabilities: */

	v4l2_cropcap Cropcap;
	v4l2_crop Crop;
	CLEAR (Cropcap);

	Cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == ioctl (video_device_handle, VIDIOC_CROPCAP, &Cropcap))
	{
		Crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		Crop.c = Cropcap.defrect; /* reset to default */
		if (-1 == ioctl (video_device_handle, VIDIOC_S_CROP, &Crop))
		{
			switch (errno)
			{
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	}
	else
	{
		/* Errors ignored. */
	}

	/* Select video input, video standard and tune here. */

	v4l2_format	Fmt;
	bool		set_format = true;

	CLEAR (Fmt);
	Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	Fmt.fmt.pix.width = ImagerProps.width;
	Fmt.fmt.pix.height = ImagerProps.height;
	Fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
	Fmt.fmt.pix.bytesperline = Fmt.fmt.pix.width;
	Fmt.fmt.pix.sizeimage = Fmt.fmt.pix.bytesperline * Fmt.fmt.pix.height;

	DBG_OUT1("Try format...\n");
	if (-1 == ioctl (video_device_handle, VIDIOC_TRY_FMT, &Fmt))
	{
		DBG_OUT1("VIDIOC_TRY_FMT error (%d) %s\n", errno, strerror (errno));
		
		/* Well, VIDIOC_TRY_FMT is optional ... */
		DBG_OUT1("Set format...\n");
		if (-1 == ioctl (video_device_handle, VIDIOC_S_FMT, &Fmt))
		{
			KIL_ERR("VIDIOC_S_FMT error (%d) %s\n", errno, strerror (errno));
			return (-errno);
		}

		set_format = false;
	}

	DBG_OUT1("Video format: %08x %ux%u (stride %u) buffer size %u\n",
		Fmt.fmt.pix.pixelformat,
		Fmt.fmt.pix.width, Fmt.fmt.pix.height, Fmt.fmt.pix.bytesperline,
		Fmt.fmt.pix.sizeimage);

	/* Check format */
	if( Fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_GREY )
	{
		KIL_ERR("Sorry, V4L2_PIX_FMT_GREY is not supported\n", 0);
		return -EINVAL;
	}

#if 0	// FIXME
	if( Fmt.fmt.pix.bytesperline > Fmt.fmt.pix.width )
	{
		Fmt.fmt.pix.width -= Fmt.fmt.pix.bytesperline - Fmt.fmt.pix.width;
		Fmt.fmt.pix.bytesperline = Fmt.fmt.pix.width;
		Fmt.fmt.pix.sizeimage = Fmt.fmt.pix.bytesperline * Fmt.fmt.pix.width;
		set_format = true;
	}
#endif
		
	if( set_format )
	{
		DBG_OUT1("Set format...\n");
		if (-1 == ioctl (video_device_handle, VIDIOC_S_FMT, &Fmt))
		{
			KIL_ERR("VIDIOC_S_FMT error (%d) %s\n", errno, strerror (errno));
			return (-errno);
		}

		DBG_OUT1("Video set format: %08x %ux%u (stride %u) buffer size %u\n",
			Fmt.fmt.pix.pixelformat,
			Fmt.fmt.pix.width, Fmt.fmt.pix.height, Fmt.fmt.pix.bytesperline,
			Fmt.fmt.pix.sizeimage);
	}

	ImagerProps.width = Fmt.fmt.pix.bytesperline;
	ImagerProps.height = Fmt.fmt.pix.height;

#ifdef MEDIA_DEVICE
	media_setup_capture_subdev(Fmt.fmt.pix.width, Fmt.fmt.pix.height);
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
	DBG_FUNC();

	int	ret;

	ASSERT(IOCTL_DEVICE_HANDLE>=0);
	ret = ioctl(IOCTL_DEVICE_HANDLE, request, parg);

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
	DBG_FUNC();

	int result;

	/* Open video device */
	DBG_OUT1("CALL camera_open...\n");
	result = camera_open();
	if(result < 0) // TODO: check me returns errors return +, but checking - here
	{
		KIL_ERR("failed to open camera, error=%d\n", result);
		return result;
	}

	/* Create thread control objects */
	bStreamOn = false;
	bExitThread = false;
	pthread_mutex_init(&accessDeque, NULL);
	pthread_cond_init(&cvDeque, NULL);

	result = pthread_create(&dequeue_thread_id, NULL, &dequeue_thread, NULL);
	DBG_OUT1("create dequeue_thread result=%d thread_id=%d\n", result, dequeue_thread_id);
	if(result < 0)
	{
		KIL_ERR("Failed to create 'dequeue_thread' Error=%d\n", result);
		return result;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Request v4l2 buffers
/*! 
 \param num_buffers
 \return number of buffers allocated on success, otherwise error code
*/
int camera_allocate_buffers(int num_buffers)
{
	DBG_FUNC();

	struct v4l2_requestbuffers	req;

	cbBufLength = 0;

	// Request v4l2 buffers
	CLEAR (req);
	req.count = num_buffers;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = memtype;
	if (-1 == ioctl (video_device_handle, VIDIOC_REQBUFS, &req))
	{
		KIL_ERR("VIDIOC_REQBUFS error (%d) %s", errno, strerror (errno));
		return (-errno);
	}

	DBG_OUT1("VIDIOC_REQBUFS: count=%i\n", req.count);

	return req.count;
}

///////////////////////////////////////////////////////////////////////////////
//! Query v4l2 buffer
/*! 
 \param index
 \return buffer pointer on success, otherwise 0
*/
void* camera_query_buffer(int index)
{
	DBG_FUNC();

	v4l2_buffer vBuf;
	void	*ptr;

	CLEAR (vBuf);
	vBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vBuf.memory = V4L2_MEMORY_MMAP;
	vBuf.index = index;
	if (-1 == ioctl (video_device_handle, VIDIOC_QUERYBUF, &vBuf))
	{
		DBG_OUT1("VIDIOC_QUERYBUF error (%d) %s\n", errno, strerror (errno));
		return NULL;
	}

	if( !cbBufLength ) cbBufLength = vBuf.length;

	ptr = mmap(NULL, cbBufLength, PROT_READ | PROT_WRITE, MAP_SHARED, video_device_handle, vBuf.m.offset);
	if( ptr == MAP_FAILED )
	{
		DBG_OUT1("mmap error (%d) %s\n", errno, strerror (errno));
		return NULL;
	}

	return ptr;
}

///////////////////////////////////////////////////////////////////////////////
//! Unmap buffer
/*! 
 \param index
 \param data_ptr
 \return o on success, otherwise error code
*/
int camera_discard_buffer(int index, void *data_ptr)
{

	DBG_FUNC();

	munmap(data_ptr, cbBufLength);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Dequeue the buffer
/*! 
 \param index
 \return buffer on success, otherwise error code
*/
static int camera_dequeue_buffer(void)
{
	DBG_FUNC();

	v4l2_buffer vBuf;
	int	rtn;

	CLEAR (vBuf);
	vBuf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vBuf.memory	= memtype;

	DBG_OUT1("Do VIDIOC_DQBUF ioctl\n");
	//  Blocks if no buffer is completed
	rtn = ioctl (video_device_handle, VIDIOC_DQBUF, &vBuf);
	DBG_V4L2_Buffer(&vBuf);
	if( rtn < 0 )
	{
		DBG_OUT1("DQBUF error (%d) %s\n", errno, strerror (errno));
		return rtn;
	}

	pthread_mutex_lock(&accessDeque);
	if( cBufQueue ) --cBufQueue;
	pthread_mutex_unlock(&accessDeque);

	DBG_OUT1("%s: Dequeue index = %d\n", __FUNCTION__, vBuf.index);

	/* Return */
	return vBuf.index;
}

///////////////////////////////////////////////////////////////////////////////
//! Enqueue the buffer
/*! 
 \param index
 \return 0 on success, otherwise error code
*/
int camera_queue_buffer(int index)
{
	DBG_FUNC(); 

	v4l2_buffer vBuf;
	int rtn;
	DBG_VAR(int value);

	/* Create v4l2 buffer object */
	CLEAR (vBuf);
	vBuf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vBuf.memory	= memtype;
	vBuf.index	= index;
	DBG_V4L2_Buffer(&vBuf);

	/* Queue Buffer (QBUF) */
	rtn = ioctl (video_device_handle, VIDIOC_QBUF, &vBuf);
	DBG_V4L2_Buffer(&vBuf);
	if (rtn < 0)
	{
		DBG_OUT1("QBUF error (%d) %s\n", errno, strerror (errno));
		return rtn;
	}

	/* Notify deque thread buffer is ready to DQBUF */
	pthread_mutex_lock(&accessDeque);
	++cBufQueue;
	DBG_OUT1("Notify thread buffer is Queued sem_val = %d\n", cBufQueue);
	pthread_cond_signal(&cvDeque);
	pthread_mutex_unlock(&accessDeque);

	/* Return */
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//! Dequeue thread - wait for QBUF, then DQBUF and notify
/*! 
 \param pParm  
 \return 1 on success 
*/
static void *dequeue_thread(void* pParm)
{
	fd_set		rfds;
	int		value;
	timeval		timeout;

	DBG_OUT1("dequeue_thread started\n");

	FD_ZERO(&rfds);
	FD_SET(video_device_handle, &rfds);

	while(1)
	{
		pthread_mutex_lock(&accessDeque);
		while( !bExitThread && (!bStreamOn || !cBufQueue) )
		{
			DBG_OUT1("Wait to dequeue... Stream%s, sem_val=%d\n", bStreamOn? "On" : "Off", cBufQueue);
			pthread_cond_wait(&cvDeque, &accessDeque);
		}
		pthread_mutex_unlock(&accessDeque);

		/* Exit thread */
		if( bExitThread )
		{
			DBG_OUT1("Exit thread...\n");
			break;
		}

		DBG_OUT1("~~~~~~~~~~~~~~~~ Wait for buffer (dequeue)...\n");
		set_timeval_ms(&timeout, DEQUE_TIMEOUT_MS);
		value = select(video_device_handle+1, &rfds, NULL, NULL, &timeout);
		DBG_OUT1("select retunred %d, wait time = %u ms\n", value, DEQUE_TIMEOUT_MS - get_timeval_ms(&timeout));
		if( value == 1 )
		{
			value = camera_dequeue_buffer();
			if( value >= 0 )
			{
				camera_dequeue_buffer_notify(value, 0);
			}
		}
		else
		{
			//TODO: handle timeout, error
		}
	}

	DBG_OUT1("!!! dequeue_thread exited\n");
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//! Start streaming
/*! 
 \return 0 on success, otherwise error code
*/
int camera_streamon()
{
	/* Stream on */
	if( !bStreamOn )
	{
		enum v4l2_buf_type type;
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		DBG_OUT1("VIDIOC_STREAMON\n");
		if (-1 == ioctl (video_device_handle, VIDIOC_STREAMON, &type))
		{
			DBG_OUT1("VIDIOC_STREAMON error (%d) %s\n", errno, strerror (errno));
			return -errno;
		}

		/* Notify thread */
		pthread_mutex_lock(&accessDeque);
		bStreamOn = true;
		pthread_cond_signal(&cvDeque);
		pthread_mutex_unlock(&accessDeque);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Stop streaming. Removes all queued buffers
/*! 
 \return 0 on success, otherwise error code 
*/
int camera_streamoff()
{
	DBG_FUNC();

	int	rtn = 0;

	if( bStreamOn )
	{
		/* Notify thread */
		pthread_mutex_lock(&accessDeque);
		bStreamOn = false;
		cBufQueue = 0;
		pthread_mutex_unlock(&accessDeque);

		/* This also removes all buffers from incoming and outgoing queues */
		enum v4l2_buf_type type;
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		DBG_OUT1("VIDIOC_STREAMOFF\n");
		if (-1 == ioctl(video_device_handle, VIDIOC_STREAMOFF, &type))
		{
			DBG_OUT1("VIDIOC_STREAMOFF error (%d) %s\n", errno, strerror (errno));
			rtn = -errno;
		}
	}

	return rtn;
}

///////////////////////////////////////////////////////////////////////////////
//! Deinitialize the camera interface
/*! 
*/
void camera_deinit(void)
{
	DBG_FUNC();

	int result;
	
	/* Notify thread to exit */
	pthread_mutex_lock(&accessDeque);
	bExitThread = true;
	pthread_cond_signal(&cvDeque);
	pthread_mutex_unlock(&accessDeque);

	/* Kill thread */
	result = pthread_join(dequeue_thread_id, NULL);
	if(result < 0)
	{
		KIL_ERR("failed to join thread 'dequeue_thread' (thread_id=%d), error =%d\n", dequeue_thread_id, result);
	}

	camera_streamoff();

	/* Close handles */
	if( video_device_handle != INVALID_HANDLE )
	{
		::close (video_device_handle);
		video_device_handle = INVALID_HANDLE;
	}

#ifdef VIDEO_SUBDEV
	if( video_subdev_handle != INVALID_HANDLE )
	{
		::close (video_subdev_handle);
		video_subdev_handle = INVALID_HANDLE;
	}
#endif
}

