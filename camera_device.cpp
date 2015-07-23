//! \file
#if !defined(NO_DEBUG) && !defined(CAMERA_DEVICE_DEBUG_ENABLED)
#warning "CAMERA_DEVICE_DEBUG_ENABLED NOT DEFINED, therefore no camera_device debug"
#define NO_DEBUG    1
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

#include "CameraScanner.h"

#define buffer_handle_t    android_buffer_handle_t
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#undef buffer_handle_t

#include "CommonInclude.h"
#include "camera_device.h"
#include "queue.h"
#include <drv/sensor_hal.h>


#define LOG_TAG "CameraDevice"
using namespace android;
//using namespace std;

#if (NUM_BUFFERS<6)
#error We need at least 5 buffers
#endif

// Image Buffer Structure (internal to KIL)
typedef struct ImageBuffer
{
    void    *start;        //!< Points to buffer with image
    size_t    length;        //!< Size of buffer
    bool    InApp;        //!< Help with buffer debugging
    int        handle;        //!< Handle to the scan driver
    bool    IsQueued;    //!< Is this buffer queued?
    bool    IsReleased;    //!< Is this buffer released by Scan Driver?
} ImageBuffer_t;

struct hsm_engine_properties    ImagerProps = { HSM_PROPERTIES_V1 };

class ScannerListener : public CameraListener
{
public:
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2){}
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
    virtual void copyAndPost(const sp<IMemory>& dataPtr, int msgType, ImageBuffer* callbackBuffers);
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr){}
};

// Local Variables:
static sp<CameraScanner>    pCamera = NULL;        //!< Camera Object
static ScannerListener  scannerListener;        //!< Camera callbacks
static ImageBuffer_t    aBuffers[NUM_BUFFERS];    //!< Keep track of image buffers
static int        nNumBuffers = 0;    //!< Amount of image buffers we can use
static queue        bufferQueue;        //!< Buffer queue
static pthread_mutex_t    accessQueue;        //!< Protect queue

static vsync_notify_t    pfVsyncNotify;        //!< Vsync callback
static void        *pDataVsync;        //!< Vsync data

static void dumpImg(
    char *addr,
    int32_t size,
    char const * const tag,
    char const * const filetype,
    uint32_t filenum)
{
    char fileName[64];
    sprintf(fileName, "/sdcard/%s_%d.%s", tag, filenum, filetype);
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        DBG_OUT1("fail to open file to save img: %s", fileName);
        return;
    }

    fwrite(addr, 1, size, fp);
    fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////
//! Gets the pointer to the buffer based on the index
/*! 
 \param index
 \return pointer to buffer
*/
void *camera_get_buffer_pointer(int index, int handle)
{
    ASSERT(index >=0);
    ASSERT(index < nNumBuffers);
    if( (index >=0) && (index < nNumBuffers) )
    {
        aBuffers[index].handle = handle;
        return aBuffers[index].start;
    }
    else
    {
        return NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the image buffers (KIL buffers)
/*! 
 \return 0 on success, otherwise error code
*/
int camera_init_image_buffers(int num_buffers)
{
    DBG_FUNC();

    for (nNumBuffers = 0; nNumBuffers < num_buffers; ++nNumBuffers)
    {
        DBG_OUT1("Init Buffer[%d]\n", nNumBuffers);

        aBuffers[nNumBuffers].InApp = false;
        aBuffers[nNumBuffers].length = ImagerProps.height * ImagerProps.width;
        aBuffers[nNumBuffers].start = malloc(aBuffers[nNumBuffers].length);
        aBuffers[nNumBuffers].handle = nNumBuffers + 1; // off by 1
        aBuffers[nNumBuffers].IsQueued = false;
        aBuffers[nNumBuffers].IsReleased = true;

        if( !aBuffers[nNumBuffers].start )
        {
            DBG_OUT1("Out of memory");
            return -ENOMEM;
        }

        DBG_OUT1("Buffer[%i] = %X, len=%i\n", nNumBuffers, aBuffers[nNumBuffers].start, aBuffers[nNumBuffers].length);
    }

    return nNumBuffers;
}

///////////////////////////////////////////////////////////////////////////////
//! Open the handle to the camera device
/*! 
 \return 0 on success, otherwise error code
*/
int camera_open(void)
{
    DBG_FUNC();
    CameraParameters    params;
    status_t        rtn;

    /* Open video device */

    pCamera = CameraScanner::connect(CAMERA_ID);
    if( pCamera == 0 )
    {
        KIL_ERR("Cannot connect to camera %d: %i, %s\n", CAMERA_ID, errno, strerror(errno));
        return -errno;
    }

    DBG_OUT1("Camera: %d\n", CAMERA_ID);

    pCamera->setScannerListener(&scannerListener);    //enable data callback
    #if 0
    ImagerProps.width=752;
    ImagerProps.height=480;
    ImagerProps.i2c_addr_sensor=MT9V022_I2C_ADDR;
    #endif
    if( 0 != camera_ioctl(HSM_GET_PROPERTIES, &ImagerProps) )
    {
        pCamera->disconnect();
        KIL_ERR("HSM_GET_PROPERTIES error %d\n", errno);
        return -1;
    }

    params.unflatten(pCamera->getParameters());
    params.set("mtk-cam-mode", 1);
    params.set("scanner-mode", "on");
    DBG_OUT1("width=%d,height=%d",ImagerProps.width, ImagerProps.height);
    params.setPreviewSize(ImagerProps.width, ImagerProps.height);
//    params.setPreviewFormat("bayer8"); // FIXME: "raw"
    params.setPreviewFormat("yuv422i-yuyv"); // FIXME: "raw"
    rtn = pCamera->setParameters(params.flatten());
    if( rtn != OK )
    {
        KIL_ERR("setParameters error %d\n", rtn);
    }

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
    DBG_FUNC();
    status_t    rtn;
    int32_t        cb;

    hsm_engine_properties *pScannerProps=(hsm_engine_properties *)parg;
    hsm_gpio_data *pScannerGpio = (hsm_gpio_data *)parg;
    hsm_iic_data  *pScannerRegData=(hsm_iic_data *)parg;
    hsm_iic_data* data = (hsm_iic_data *)parg;

#if 0
    DBG_OUT1("ZZZ camera_ioctl parg=0x%x\n", (int32_t)parg);
    rtn = pCamera->sendCommand(288, request, (int32_t)parg);
    switch(request){
        case HSM_GET_PROPERTIES:
            DBG_OUT1("ZZZ camera_ioctl: HSM_GET_PROPERTIES\n");
            DBG_OUT1("ZZZ camera_ioctl: HSM_GET_PROPERTIES wersion=%d,width=%d\n",pScannerProps->version,pScannerProps->width);
            //rtn = pCamera->sendCommand(288, request, (int32_t)pScannerProps);
            //return 0;
            break;
        case HSM_IIC_WRITE:
            DBG_OUT1("ZZZ camera_ioctl: HSM_IIC_WRITE\n");
            DBG_OUT1("ZZZ camera_ioctl: HSM_IIC_WRITE reg:0x%x,buf:0x%x\n",pScannerRegData->reg,*(pScannerRegData->buf));
            //rtn = pCamera->sendCommand(288, request, (int32_t)pScannerRegData);
            //return 0;
            break;
        case HSM_IIC_READ:
            DBG_OUT1("ZZZ camera_ioctl: HSM_IIC_READ\n");
            DBG_OUT1("ZZZ camera_ioctl========================================: %x,%x",data->buf[0],data->buf[1]);
            //DBG_OUT1("ZZZ camera_ioctl: HSM_IIC_READ reg:0x%x,buf:0x%x\n",pScannerRegData->reg,*(pScannerRegData->buf));
            break;
        case HSM_GPIO_WRITE:
            DBG_OUT1("ZZZ camera_ioctl: HSM_GPIO_WRITE\n");
            DBG_OUT1("ZZZ camera_ioctl: HSM_GPIO_WRITE pin:%d,val:0x%d\n",pScannerGpio->pin,pScannerGpio->value);
            //rtn = pCamera->sendCommand(288, request, (int32_t)pScannerGpio);
            //return 0;
            break;
        case HSM_GPIO_READ:
            DBG_OUT1("ZZZ camera_ioctl: HSM_GPIO_READ\n");
            break;
        case HSM_IIC_TRANSFER:
            DBG_OUT1("ZZZ camera_ioctl: HSM_IIC_TRANSFER\n");
            break;
        case HSM_STATUS_READ:
            DBG_OUT1("ZZZ camera_ioctl: HSM_STATUS_READ\n");
            break;
        case HSM_SUPPLY_WRITE:
            DBG_OUT1("ZZZ camera_ioctl: HSM_SUPPLY_WRITE\n");
            break;
        case MT9V022_I2C_ADDR:
            break;
        case HSM_SUPPLY_READ:
            DBG_OUT1("ZZZ camera_ioctl: HSM_SUPPLY_READ\n");
            break;
        default:
            KIL_ERR("ZZZ camera_ioctl: Command ID = %d is undefined\n",request);

    }
#else
    cb = _IOC_SIZE(request);
    rtn = pCamera->sendCommand(288, request, (int32_t)parg);
#endif
    if( rtn != OK )
    {
        KIL_ERR("sendCommand failed, error = %d\n", rtn);
        return (rtn < 0)? rtn : -1;
    }
    return 0;
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
    LOGD("CALL camera_open");
    DBG_OUT1("CALL camera_open...\n");
    result = camera_open();
    if( result < 0 ) // TODO: check me returns errors return +, but checking - here
    {
        KIL_ERR("failed to open camera, error=%d\n", result);
        return result;
    }
    /* Initialize image buffers */
    DBG_OUT1("Call camera_init_image_buffers...\n");
    result = camera_init_image_buffers(NUM_BUFFERS);
    if( result < 0 )
    {
        KIL_ERR("failed to init image buffers, error=%d\n", result);
        return result;
    }

    /* Create buffer queue */
    DBG_OUT1("Create buffer queue...\n");
    bufferQueue = createQueue();
    pthread_mutex_init(&accessQueue, NULL);

    return 0;
}

static int i = 0;
void ScannerListener::postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
  // VM pointer will be NULL if object is released
    if( msgType & CAMERA_MSG_PREVIEW_FRAME )
    {
        int    nCurrentBuffer;    // Current buffer
        int    nLastBuffer;    // Last buffer

        if( camera_dequeue_buffer(nCurrentBuffer) >= 0 )
        {
            DBG_OUT1("Call camera_dequeue_buffer nCurrentBuffer=%d\n", nCurrentBuffer);

            nLastBuffer = nCurrentBuffer; // set last buffer

            DBG_OUT1("~~~~~~~~~~~ Received buffer!!\n");
            copyAndPost(dataPtr, msgType, &aBuffers[nLastBuffer]);
            //dumpImg((char*)aBuffers[nLastBuffer].start,aBuffers[nLastBuffer].length,"hy_buffer","raw",i++);
            /* Notify Vsync */
            DBG_OUT1("call back to scan driver\n");
            DBG_OUT1("nLastBuffer=%d, handle=%d\n", nLastBuffer, aBuffers[nLastBuffer].handle);
            DBG_OUT1("vsync_cb_data=0x%x\n", pDataVsync);
            DBG_OUT1("BEFORE Callback\n");
            pfVsyncNotify(aBuffers[nLastBuffer].handle, pDataVsync);
            DBG_OUT1("AFTER Callback\n");
        }
    }
}

void ScannerListener::copyAndPost(const sp<IMemory>& dataPtr, int msgType, ImageBuffer* callbackBuffers) {

    if (dataPtr != NULL) {
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
        DBG_OUT1("copyAndPost: off=%ld, size=%d", offset, size);
        uint8_t *heapBase = (uint8_t*)heap->base();

        if (heapBase != NULL) {
            const char* data = reinterpret_cast<const char*>(heapBase + offset);
            DBG_OUT1("backbuffer length=%d",callbackBuffers->length);
            if (callbackBuffers != NULL && callbackBuffers->start != NULL) {
                DBG_OUT1("copy buffer to callbackbuffer !");
                memset(callbackBuffers->start,0,callbackBuffers->length);
                memcpy(callbackBuffers->start,data, callbackBuffers->length);
            }
        } else {
            DBG_OUT1("image heap is NULL");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//! Dequeue the buffer
/*! 
 \param index
 \return 0 on success, otherwise error code
 Blocks if no buffer is completed
*/
int camera_dequeue_buffer(int &index)
{
    DBG_FUNC();

    /* Pop the buffer off the buffer queue */
    pthread_mutex_lock(&accessQueue);
    index = bufferQueue.size? bufferQueue.pop(&bufferQueue) : -1;
    pthread_mutex_unlock(&accessQueue);

    if( index < 0 )
    {
        return -1;
    }

    /* Checks */
    ASSERT(index<nNumBuffers);        // Valid index

    DBG_OUT1("Dequeue index = %d\n", index);

    aBuffers[index].InApp = true;

    /* Set flag to indicate the buffer is no long queued */
    aBuffers[index].IsQueued = false;

#if (CAPTURE_MODE == 1) || (CAPTURE_MODE == 4) || (CAPTURE_MODE == 6)
    /* It might be ok to queue this buffer:
         At this point, the KIL is done with the buffer, but the
         Scan Driver likely still has control of the buffer */
    if( aBuffers[index].IsReleased ) camera_queue_buffer(index);
    DBG_OUT1("%s: index = %d queued\n", __FUNCTION__, index);
#endif

    /* Debug store image */
    //DBG_STORE_IMAGE((unsigned char *) camera_get_buffer_pointer(index), DBG_STAMP_HWL);

    /* Return */
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the capture interface
/*! 
 \param index
 \return 0 on success, otherwise error code
*/
int camera_queue_buffer(int index)
{
    DBG_FUNC();

    /* Checks */
    DBG_OUT1("%s index=%i, nNumBuffers=%i\n", __FUNCTION__ ,index, nNumBuffers);
    ASSERT (index >=0);                    // valid index

    /* Push the buffer onto the buffer queue */
    pthread_mutex_lock(&accessQueue);
    bufferQueue.push(&bufferQueue, index);
    pthread_mutex_unlock(&accessQueue);

    /* Indicate that this buffer is (1) queued and (2) not released */
    aBuffers[index].IsQueued = true;
    aBuffers[index].IsReleased = false;

    /* Return */
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Gets the next buffer from the queue
/*!  
 \return next buffer off queue
*/
int camera_get_next_buffer()
{
    int    index;

    pthread_mutex_lock(&accessQueue);
    index = bufferQueue.size? bufferQueue.peek(&bufferQueue) : -1;
    pthread_mutex_unlock(&accessQueue);

    DBG_OUT1("%s: Next buffer in Queue is %d (to be Dequeue by capture)\n", __FUNCTION__, index);

    return    index;
}

///////////////////////////////////////////////////////////////////////////////
//! Called from the Scan Driver to release a specific buffer (control of the buffer passed back to the KIL)
/*! 
 \param index 
*/
void camera_release_buffer(int index)
{
    /* Release this buffer  */
    DBG_OUT1("%s: index = %d released from SD\n", __FUNCTION__, index);
    ASSERT(index<nNumBuffers);        // Valid index
    ASSERT(!aBuffers[index].IsQueued);
    aBuffers[index].IsReleased = true;

#if (CAPTURE_MODE == 1) || (CAPTURE_MODE == 4) || (CAPTURE_MODE == 6)
    /* Queue this buffer */
    camera_queue_buffer(index);
    DBG_OUT1("%s: index = %d queued\n", __FUNCTION__, index);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//! Start scanning to reset/setup capture interface before scans
/*! 
 \return next capture buffer that is to be filled 
*/
int camera_start_capture(void)
{
    int        index = -1;
    status_t    rtn;

#if (CAPTURE_MODE == 1) || (CAPTURE_MODE == 4) || (CAPTURE_MODE == 6)
    /* Itterate through all buffers */
    for( index = 0; index < nNumBuffers; index++ )
    {
        /* Is this buffer not queued && released by SD, then queue */
        if( aBuffers[index].IsQueued == false && aBuffers[index].IsReleased == true )
        {
            DBG_OUT1("%s: Queue buffer %d\n", __FUNCTION__, index);
            camera_queue_buffer(index);
        }
    }
#endif

    /* Get the next buffer (off the queue) */
    index = camera_get_next_buffer();

    /* Checks */
    ASSERT(index>=0);        // Valid index
    ASSERT(index<nNumBuffers);    // Valid index
    DBG_FUNC();
    /* Stream on */
    if( !pCamera->previewEnabled() )
    {
        DBG_OUT1("setPreviewCallbackFlag");
        pCamera->setPreviewCallbackFlag(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);
        rtn = pCamera->startPreview();
        if( rtn != OK )
        {
            DBG_OUT1("startPreview error (%d)\n", rtn);
            return -1;
        }
    }

    /* Return (index) */
    DBG_OUT1("%s: returns index = %d\n", __FUNCTION__, index);
    return index;
}

///////////////////////////////////////////////////////////////////////////////
//! Stop scanning to reset/setup capture interface after scans
/*! 
 \return 0 on success 
*/
int camera_stop_capture(void)
{
    DBG_FUNC();

    pCamera->stopPreview();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Register vsync notification with the scan driver (callback)
/*! 
 \param pfNotification 
 \param data  
 \return 1 on success 
*/
unsigned long camera_register_vsync_notification(vsync_notify_t pfNotification, void * data)
{
    DBG_FUNC();
    pfVsyncNotify = pfNotification;
    pDataVsync = data;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//! Deinitialize the camera interface
/*! 
*/
void camera_deinit()
{
    DBG_FUNC();

    /* Release camera */
    if( pCamera != 0 )
    {
        pCamera->stopPreview();
        DBG_OUT("Disconnect Camera");
        pCamera->disconnect();
        pCamera.clear();
        pCamera = NULL;
    }

    /* Clear queue */
    pthread_mutex_lock(&accessQueue);
    while( bufferQueue.size )
    {
        bufferQueue.pop(&bufferQueue);
    }
    pthread_mutex_unlock(&accessQueue);
    pthread_mutex_destroy(&accessQueue);

    /* Free buffers */
    for (unsigned int i = 0; i < nNumBuffers; ++i)
    {
        free(aBuffers[i].start);
    }

    nNumBuffers = 0;
}
