/*
 * Camera.h
 *
 *  Created on: 2014Äê4ÔÂ3ÈÕ
 *      Author: Administrator
 */

#ifndef CAMERA_SCANNER_H_
#define CAMERA_SCANNER_H_
#include <utils/Mutex.h>
#include "CameraHardwareInterface.h"
#include <gui/SurfaceTextureClient.h>
#include <gui/ISurface.h>

using namespace android;

class CameraScanner : public RefBase {
public:
    CameraScanner();
    virtual ~CameraScanner();
    virtual void            disconnect();
    static sp<CameraScanner>  connect(int cameraId);
    virtual status_t        lock();
    virtual status_t        unlock();
    //virtual status_t        setPreviewDisplay(const sp<Surface>& surface);
    virtual status_t        setPreviewWindow(const sp<IBinder>& binder,const sp<ANativeWindow>& window);
    virtual status_t        setPreviewTexture(const sp<ISurfaceTexture>& surfaceTexture);
    virtual void            setPreviewCallbackFlag(int flag);
    virtual status_t        startPreview();
    virtual void            stopPreview();
    virtual bool            previewEnabled();
    virtual status_t        storeMetaDataInBuffers(bool enabled);
    virtual status_t        startRecording();
    virtual void            stopRecording();
    virtual status_t        autoFocus();
    virtual status_t        cancelAutoFocus();
    virtual status_t        takePicture(int msgType);
    virtual status_t        setParameters(const String8& params);
    virtual String8         getParameters();
    virtual status_t        sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

    void             		setScannerListener(const sp<CameraListener>& listener);
    void                    disconnectWindow(const sp<ANativeWindow>& window);
    // these are static callback functions
    static void             notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2, void* user);
    static void             dataCallback(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata, void* user);
    static void             dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr, void* user);
private:
    status_t                startPreviewMode();
    void                    initialize(int cameraId);
    void                    enableMsgType(int32_t msgType);
    void                    disableMsgType(int32_t msgType);
    volatile int32_t        mMsgEnabled;
    mutable Mutex           mLock;
    sp<CameraListener>  	mListener;
    static sp<CameraScanner>   sCameraScanner;
private:
    int                     mPreviewCallbackFlag;
    sp<ANativeWindow>       mPreviewWindow;
    sp<IBinder>             mSurface;
    int                     mOrientation;

    // If the user want us to return a copy of the preview frame (instead
    // of the original one), we allocate mPreviewBuffer and reuse it if possible.
    sp<MemoryHeapBase>      mPreviewBuffer;
    sp<CameraHardwareInterface> mHardware;
    camera_module_t*        mModule;
    int 					mCameraId;
};

#endif /* CAMERA_H_ */
