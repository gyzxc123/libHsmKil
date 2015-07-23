#define LOG_TAG "HWL"
#if !defined(NO_DEBUG) && !defined(HWLAYER_SKEL_DEBUG_ENABLED)
#warning "HWLAYER_SKEL_DEBUG_ENABLED NOT DEFIND, therefore no hwlayer_skel debug"
#define NO_DEBUG	1
#endif

#include <unistd.h>
#include <time.h>

#include "CommonInclude.h"
#include "HWLayer_Skel.h"
#include "HWLayerN5600.h"
#include "HWLayerIT5000.h"

#include "camera_device.h"
#include "capture.h"
#include "gpio.h"
#include "i2c.h"

//#include <mach/mt_gpio.h>


#if CAMERA_FOR_SCANNER
//#include <linux/ioctl.h>
#include <fcntl.h>

#include <sys/ioctl.h>



#define CMD_FLAG  's'
#define SCAN_ON		_IOR(CMD_FLAG, 0x00000000, __u32)
#define SCAN_OFF	_IOR(CMD_FLAG, 0x00000001, __u32)

#define AIM_ON		_IOR(CMD_FLAG, 0x00000003, __u32)
#define AIM_OFF	_IOR(CMD_FLAG, 0x00000004, __u32)
#define ILLUM_ON		_IOR(CMD_FLAG, 0x00000005, __u32)
#define ILLUM_OFF	_IOR(CMD_FLAG, 0x00000006, __u32)


//#define ILLUM_ON		_IOR(CMD_FLAG, 0x00000005 __u32)
//#define ILLUM_OFF	_IOR(CMD_FLAG, 0x00000006, __u32)



#define SE955_CHANNEL_NAME "/dev/se955" 

int fd_scanner = -1;

static int aimoff_enable = 0;

#endif


#define UNIMPL 0

inline buffer_handle_t index2handle(int index)	{ return (buffer_handle_t)(index + 1); }	// off by 1 with SD
inline int handle2index(buffer_handle_t handle) { return (int)(handle - 1); }

inline void WaitMilliseconds(unsigned int mSec) { usleep(mSec*1000); }

// Variables:
static int	capture_mode = DEF_CAPTURE_MODE;
static HwlBase	*gp_Hwl = NULL;

static bool bSessionOpen = false;


#define CRT_FINI 1		// until I know a better way
#ifdef CRT_FINI
///////////////////////////////////////////////////////////////////////////////
//! Emergency cleanup.
/*!
 * The class CFini is responsible to do an emergency cleanup if the user of the lib
 * forgets to call DeinitHWLayer.
 * The special attributes tell the linker to place the ctor/dtor in a table for static objects.
 * So the dtor is called before the lib gets unloaded.
 * Theoretically we could expect the OS does all the resource cleanup, but we want to play nice and save.
 * Background: In initfini.c, the C-Lib defines the _init() and _fini() functions that take care of
 * calling the static objects.
 * */

class CFini_Kil
{
public:
	CFini_Kil()
	{
		DBG_FUNC();
	}

	~CFini_Kil()
	{
		DBG_FUNC();
		DeinitHWLayer(gp_Hwl);
	}
};

static CFini_Kil Cleaner;

#else
// This function gets called just before the lib is unloaded
// http://linux.die.net/man/3/dlclose
void _fini()
{
	DBG_FUNC();
	DeinitHWLayer(gp_Hwl);
}
#endif // CRT_FINI

// This is the first function the Scan driver ever calls.  Any
// initializtion the HW needs to do should be in this function or in
// the HwlBase constuctor.
IHwl * InitHWLayer(void * context)
{
	DBG_FUNC();

	int	result;

	if( gp_Hwl && bSessionOpen )
	{
		DBG_OUT1("%s is called %d times", __FUNCTION__, 2);
		// TODO: Reinit?
		return gp_Hwl;
	}

	/* Initialize the camera device */
	result = camera_init();
	if( result < 0 )
	{
		KIL_ERR("::::::::::Camera_init Failed::::::::::::::, error=%d\n", result);
		return NULL;
	}

#ifdef ANDROID_CAMERA_ID
	capture_mode = 4;
#else
	capture_mode = (ImagerProps.i2c_addr_sensor == MT9V022_I2C_ADDR)? 4 : 6;
#endif

	// Initialize the capture interface
	capture_mode = capture_init(capture_mode);
	if( capture_mode < 0 )
	{
		camera_deinit();
		return NULL;
	}

	bSessionOpen = true;

	if( !gp_Hwl )
	{
		switch( ImagerProps.i2c_addr_sensor )
		{
		case EV76C454_I2C_ADDR :
			gp_Hwl = new HwlN5600();
			break;

	case MT9V022_I2C_ADDR :
		gp_Hwl = new HwlIT5000();
		break;

		default :
			gp_Hwl = new HwlBase();
		}
	}

	return gp_Hwl;
}

// If the scan driver is unloaded it will call DeinitHWLayer.  Any
// clean up required by the HW Layer should be in this function or in
// HwlBase destructor.
void DeinitHWLayer(IHwl * p_hwlayer)
{
	DBG_FUNC();

	capture_deinit();
	camera_deinit();

	if( gp_Hwl )
	{
		gp_Hwl->CloseSession();

		delete gp_Hwl;
		gp_Hwl = NULL;
	}
}

void GetHWLayerRevision(char * pszRev, size_t BufferSize)
{
	DBG_FUNC();

	if( pszRev )
	{
		// for now a hardcoded build date.
		strncpy(pszRev, __DATE__, BufferSize);
	}
}

///////////////////////////////////////////////////////////////////////////////
//! HwlBase implementation

HwlBase :: HwlBase()
{
	DBG_FUNC();
}

HwlBase :: ~HwlBase()
{
	DBG_FUNC();
}

const char * HwlBase :: EngineType()
{
	DBG_FUNC();
	return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////
//! Return the image buffer pointers structure to the Scan Driver
/*! 
 \param n 
 \return pointer to image buffers
*/
const Image_buffer_pointers * HwlBase :: GetImageBuffers(size_t & n)
{
	// This is very different from the way HW Layer's used to work.
	// This function should return an array of Image_buffer_pointers.
	// There should one entry in the array for each image buffer
	// supported by the HW Layer.  This function should set 'n' to the
	// number of buffers.

	DBG_FUNC();

	static Image_buffer_pointers	sd_image_buffers[NUM_BUFFERS];	//!< Buffer list compatible to the SD
	int	i;

	for( i = 0; i < NUM_BUFFERS; i++ )
	{
		unsigned char	*ptr = (unsigned char*) capture_get_buffer_pointer(i);
		buffer_handle_t	handle;
		
		if( !ptr ) break;

		handle = index2handle(i);
		capture_set_buffer_handle(i, handle);

		DBG_OUT1("init buffer %d, 0x%x\n", i, ptr);
		sd_image_buffers[i].p_cached = ptr;
		sd_image_buffers[i].p_uncached = ptr;
		sd_image_buffers[i].handle = handle;
	}
	n = i;
	return sd_image_buffers;
}

void HwlBase :: 
RegisterVsyncNotification(  void(*_pf_vsync_notif)(buffer_handle_t, void *), void * data)
{
	DBG_FUNC();
	capture_register_vsync_notification(_pf_vsync_notif, data);
}

unsigned HwlBase :: GetScanHeight(void)
{
	DBG_FUNC();
	return ImagerProps.height;
}

unsigned HwlBase :: GetScanWidth(void)
{
	DBG_FUNC();
	return ImagerProps.width;
}

void HwlBase :: AimOn(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER

	//ioctl(fd_scanner, AIM_ON);
		return ;
	#endif
	gpio_set_value(HSM_GPIO_AIMER, 1); // aimer on
}

void HwlBase :: AimOff(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
/*
	if(aimoff_enable == 1)
	{
		ioctl(fd_scanner, AIM_OFF);
		aimoff_enable = 0;
	}*/
		return ;
	#endif
	gpio_set_value(HSM_GPIO_AIMER, 0); // aimer off
}

void HwlBase :: AimOnZltd(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER

	ioctl(fd_scanner, AIM_ON);
		return ;
	#endif
	gpio_set_value(HSM_GPIO_AIMER, 1); // aimer on
}

void HwlBase :: AimOffZltd(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER

	if(aimoff_enable == 1)
	{
		ioctl(fd_scanner, AIM_OFF);
		aimoff_enable = 0;
	}
		return ;
	#endif
	gpio_set_value(HSM_GPIO_AIMER, 0); // aimer off
}

void HwlBase :: IllumConfig(unsigned p)
{
	DBG_FUNC();
	DBG_OUT1("%s Unimplemented\n", __FUNCTION__);
}

void HwlBase :: IllumOn(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER

	//ioctl(fd_scanner, ILLUM_ON);
		return ;
	#endif
	gpio_set_value(HSM_GPIO_ILLUMINATOR, 1); // illumination on
}

void HwlBase :: IllumOff(void)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	int rtn = 0;

	//rtn = ioctl(fd_scanner, ILLUM_OFF);
	LOGE("ImagerPowerUp rtn=%d\n ",rtn);
		return ;
	#endif
	gpio_set_value(HSM_GPIO_ILLUMINATOR, 0); // illumination off
}

bool HwlBase :: ImagerPowerUp(void)
{
	DBG_FUNC();

#ifdef CAMERA_FOR_SCANNER
	
//#if CAMERA_FOR_SCANNER
			
//#endif

		return true;
#endif

	

	gpio_set_value(HSM_GPIO_ENGINE_RESET, 0); // take the engine out of reset
	gpio_set_value(HSM_GPIO_POWER_ENABLE, 1); // raise power enable

	WaitMilliseconds(30); // provide enough time for the engine to become stable
		
	return true;
}

bool HwlBase :: ImagerPowerDown(void)
{
	DBG_FUNC();
#ifdef CAMERA_FOR_SCANNER
		return true;
#endif

	

	gpio_set_value(HSM_GPIO_ENGINE_RESET, 1); // put engine out of reset
	gpio_set_value(HSM_GPIO_POWER_ENABLE, 0); // lower power enable

	return true;
}

bool HwlBase :: ImagerPowerOn()
{
	DBG_FUNC();
#ifdef CAMERA_FOR_SCANNER
		
	//#if CAMERA_FOR_SCANNER
				fd_scanner = open(SE955_CHANNEL_NAME, O_RDONLY);
	
				LOGE("ImagerPowerUp fd_scanner=%d\n ",fd_scanner);
	
				if(fd_scanner < 0) {
					LOGE("ImagerPowerUp fd_scanner fail ");
				}
	//#endif
			AimOnZltd();
			return true;
#endif

	return gpio_power_on();
}

bool HwlBase :: ImagerPowerOff()
{
	DBG_FUNC();

	
	#ifdef CAMERA_FOR_SCANNER
		//DeinitHWLayer(gp_Hwl);
		//capture_stop_preview();
		aimoff_enable = 1;
		AimOffZltd();
		aimoff_enable = 0;
		close(fd_scanner);
		return true;
	#endif

	return gpio_power_off();
}

void HwlBase :: ResetTransferHardware(void)
{
	// The scan driver doesn't call this at the moment.  So, you can
	// leave it blank. In the future we may make this an optional
	// function that the HWLayer can implement for performance reasons.
	DBG_FUNC();
	DBG_OUT1("%s is unimplemented\n", __FUNCTION__);
}

bool HwlBase :: EnableVsyncProcessing()
{
	DBG_FUNC();
	DBG_OUT1("%s is unimplemented\n", __FUNCTION__);
	return (UNIMPL);
}

bool HwlBase :: IsPsocSupported()
{
	DBG_FUNC();
	return (ImagerProps.i2c_addr_psoc != 0);
}

bool HwlBase :: WriteIIC_PSOC(unsigned char address, const unsigned char * p, unsigned cnt )
{
	DBG_FUNC();
	int err = i2c_write_reg(ImagerProps.i2c_addr_psoc, address, p, cnt);
	return (err < 0) ? false : true;
}

bool HwlBase :: ReadIIC_PSOC(unsigned char address, unsigned char *p, unsigned int cnt)
{
	DBG_FUNC();
	int err = i2c_read_reg(ImagerProps.i2c_addr_psoc, address, p, cnt);
	return (err < 0) ? false : true;
}


void HwlBase :: GetConfig(Config * p_config)
{
	DBG_FUNC();
	p_config->size = sizeof(Config);
	p_config->captureMode = capture_mode;

	DBG_OUT1("GetConfig captureMode is 6\n");
}

void HwlBase :: StartSnapshot()
{
	DBG_FUNC();

	/* When the HW Layer controls the capture buffer, this function is instead of
	   InitCapture to tell the HW Layer to capture an image. */
	if( (capture_mode == 1) || (capture_mode == 4) || (capture_mode == 6) )
	{
		//??? capture_start_scanning();
		capture_request_buffer();
	}
	else
	{
		KIL_ERR("%s: Error Invalid config (%d) for this function\n", __FUNCTION__, capture_mode);
	}
}

void HwlBase :: ReleaseBuffer(buffer_handle_t handle)
{
	DBG_FUNC();
	capture_release_buffer(handle2index(handle));
}

bool HwlBase :: InitCapture(buffer_handle_t handle)
{
	DBG_FUNC();
	if( (capture_mode == 2) || (capture_mode == 3) || (capture_mode == 5) || (capture_mode == 7) )
	{
		return (capture_request_buffer(handle2index(handle)) >= 0);
	}
	else
	{
		KIL_ERR("%s: Error Invalid config (%d) for this function\n", __FUNCTION__, capture_mode);
		return false;
	}
}

bool HwlBase :: StartScanning()
{
	DBG_FUNC();
	
#ifdef CAMERA_FOR_SCANNER
		//DeinitHWLayer(gp_Hwl);
		//capture_stop_preview();
		//AimOn();
		aimoff_enable = 1;
		AimOffZltd();
		aimoff_enable = 0;
		IllumOn();
		
#endif
	return capture_start_scanning();
}

bool HwlBase :: StopScanning()
{
	DBG_FUNC();

	AimOnZltd();

#ifdef ANDROID_CAMERA_ID
	return true;
#else
	return capture_stop_scanning();
#endif
}

bool HwlBase :: RequestStopScanning(StopScanningCallback_t fnCallback, void * closure)
{
	DBG_FUNC();

#ifdef ANDROID_CAMERA_ID
	capture_stop_scanning();	// Stop capture before SD stops the sensor.
#endif
	return true;
}

bool HwlBase :: OpenSession()
{
	DBG_FUNC();

	int	result;

	if( bSessionOpen )
	{
		DBG_OUT1("Session already open.\n");
		return true; // avoid doing work twice
	}

	InitHWLayer(NULL);

	return bSessionOpen;
}

void HwlBase :: CloseSession()
{
	DBG_FUNC();

	if( bSessionOpen )
	{
		capture_deinit();
		camera_deinit();
		bSessionOpen = false;	// session is closed
	}
}

