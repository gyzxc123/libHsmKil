/*
 * scanner.cpp
 *
 *  Created on: 2014Äê4ÔÂ15ÈÕ
 *      Author: Administrator
 */
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

class ScannerListener : public CameraListener
{
public:
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2){}
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
    {
    	printf("postData:==============   \r\n");
    }
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr){}
};

static sp<CameraScanner> sCameraScanner = NULL;
static ScannerListener  scannerListener;

int main(int argc, char **argv) {
	int cameraId = 0;
	int width = 800;
	int height = 600;
	int i = 0;
	char* format ="yuv422i-yuyv";
	char* formatPtr = format;
	CameraParameters	params;
	status_t		rtn;
	if (argc >1) {
		cameraId = atoi(argv[1]);
	}
	if (argc > 3) {
		width = atoi(argv[2]);
		height = atoi(argv[3]);
	}
	if (argc > 4) {
		formatPtr = argv[4];
	}
	sCameraScanner = CameraScanner::connect(cameraId);

	if( sCameraScanner == 0 )
	{
		printf("Cannot connect to camera: %i, %s\r\n", errno, strerror(errno));
		return -1;
	}

	sCameraScanner->setScannerListener(&scannerListener);
	params.unflatten(sCameraScanner->getParameters());
	params.set("mtk-cam-mode", 1);
        params.set("scanner-mode", "on");
	params.setPreviewSize(width, height);
	params.setPreviewFormat(formatPtr); // FIXME: "raw"
	rtn = sCameraScanner->setParameters(params.flatten());
	if (rtn != OK) {
		printf("setPreviewDisplay error %d\r\n", rtn);
		return rtn;
	}

	if( !sCameraScanner->previewEnabled() )
	{
		printf("setPreviewCallbackFlag\r\n");
		sCameraScanner->setPreviewCallbackFlag(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK
				|CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER);
		rtn = sCameraScanner->startPreview();
		if( rtn != OK )
		{
			printf("startPreview error (%d)\r\n", rtn);
			return -1;
		}
	}

	while(true) {
		i++;
		sleep(1);
		if (i > 5) {
			break;
		}
	}
	printf("disconnect\r\n");
	sCameraScanner->disconnect();
	return 0;
}

