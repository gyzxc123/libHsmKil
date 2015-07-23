/*
 * CameraScanner.cpp
 *
 *  Created on: 2014Äê4ÔÂ3ÈÕ
 *      Author: Administrator
 */
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include "CameraScanner.h"

#define LOG_TAG "Scanner"

using namespace android;

CameraScanner::CameraScanner() {
	mOrientation = 0;
	mPreviewWindow = 0;
	if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
                (const hw_module_t **)&mModule) < 0) {
        ALOGE("Could not load camera HAL module");
    }
}

CameraScanner::~CameraScanner() {

}

void CameraScanner::disconnect() {
	Mutex::Autolock lock(mLock);

	// Make sure disconnect() is done once and once only, whether it is called
	// from the user directly, or called by the destructor.
	if (mHardware == 0) return;

	ALOGE("hardware teardown");
	// Before destroying mHardware, we must make sure it's in the
	// idle state.
	// Turn off all messages.
	disableMsgType(CAMERA_MSG_ALL_MSGS);
	mHardware->stopPreview();
	mHardware->cancelPicture();
	// Release the hardware resources.
	mHardware->release();

	// Release the held ANativeWindow resources.
	if (mPreviewWindow != 0) {
		disconnectWindow(mPreviewWindow);
		mPreviewWindow = 0;
		mHardware->setPreviewWindow(mPreviewWindow);
	}
	mHardware->close();
	mHardware.clear();
	if (mModule) {
		mModule = NULL;
	}
}

static void CameraScanner::disconnectWindow(const sp<ANativeWindow>& window) {
    if (window != 0) {
        status_t result = native_window_api_disconnect(window.get(),
                NATIVE_WINDOW_API_CAMERA);
        if (result != NO_ERROR) {
            ALOGW("native_window_api_disconnect failed: %s (%d)", strerror(-result),
                    result);
        }
    }
}

sp<CameraScanner> CameraScanner::sCameraScanner = NULL;

static sp<CameraScanner> CameraScanner::connect(int cameraId) {
	CameraScanner* pCamera = new CameraScanner();
    if (!pCamera->mModule) {
        ALOGE("CameraScanner HAL module not loaded");
        return NULL;
    }
    pCamera->initialize(cameraId);
    sCameraScanner = pCamera;
    return pCamera;
}

void CameraScanner::setScannerListener(const sp<CameraListener>& listener) {
	mListener = listener;
}
void CameraScanner::initialize(int cameraId) {

	mCameraId = cameraId;
    char camera_device_name[10];
    status_t res;
    snprintf(camera_device_name, sizeof(camera_device_name), "%d", cameraId);

    mHardware = new CameraHardwareInterface(camera_device_name);

    struct camera_info info;
    if (mModule->get_camera_info(cameraId, &info) != OK) {
        ALOGE("Invalid camera id %d", cameraId);
        return NULL;
    }

    int numberOfCameras = mModule->get_number_of_cameras();
    ALOGE("numberOfCameras:%d",numberOfCameras);
    res = mHardware->initialize(&mModule->common);
    if (res != OK) {
        ALOGE("%s: Camera %d: unable to initialize device: %s (%d)",
                __FUNCTION__, cameraId, strerror(-res), res);
        mHardware.clear();
        return NO_INIT;
    }
    ALOGE("mCameraId=%d",mCameraId);
    mHardware->setCallbacks(notifyCallback,
            dataCallback,
            dataCallbackTimestamp,
            (void *)mCameraId);

    // Enable zoom, error, focus, and metadata messages by default
    enableMsgType(CAMERA_MSG_ERROR | CAMERA_MSG_ZOOM | CAMERA_MSG_FOCUS |
                  CAMERA_MSG_PREVIEW_METADATA | CAMERA_MSG_FOCUS_MOVE);
}

status_t CameraScanner::lock(){
	return OK;
}
status_t CameraScanner::unlock(){
	return OK;
}


status_t CameraScanner::setPreviewWindow(const sp<IBinder>& binder,
        const sp<ANativeWindow>& window) {
    Mutex::Autolock lock(mLock);

    // return if no change in surface.
    if (binder == mSurface) {
        return NO_ERROR;
    }
    status_t result;
    if (window != 0) {
        result = native_window_api_connect(window.get(), NATIVE_WINDOW_API_CAMERA);
        if (result != NO_ERROR) {
            ALOGE("native_window_api_connect failed: %s (%d)", strerror(-result),
                    result);
            return result;
        }
    }

    // If preview has been already started, register preview buffers now.
    if (mHardware->previewEnabled()) {
        if (window != 0) {
            native_window_set_scaling_mode(window.get(),
                    NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
            native_window_set_buffers_transform(window.get(), mOrientation);
            result = mHardware->setPreviewWindow(window);
        }
    }

    if (result == NO_ERROR) {
        // Everything has succeeded.  Disconnect the old window and remember the
        // new window.
        disconnectWindow(mPreviewWindow);
        mSurface = binder;
        mPreviewWindow = window;
    } else {
        // Something went wrong after we connected to the new window, so
        // disconnect here.
        disconnectWindow(window);
    }

    return result;
}

// set the Surface that the preview will use
/*status_t CameraScanner::setPreviewDisplay(const sp<Surface>& surface) {
    ALOGE("setPreviewDisplay(%p)", surface.get());

    sp<IBinder> binder(surface != 0 ? surface->asBinder() : 0);
    sp<ANativeWindow> window(surface);
    return setPreviewWindow(binder, window);
}*/

// set the SurfaceTexture that the preview will use
status_t CameraScanner::setPreviewTexture(
        const sp<ISurfaceTexture>& surfaceTexture) {
	ALOGE("setPreviewTexture(%p)", surfaceTexture.get());

    sp<IBinder> binder;
    sp<ANativeWindow> window;
    if (surfaceTexture != 0) {
        binder = surfaceTexture->asBinder();
        window = new SurfaceTextureClient(surfaceTexture);
    }
    return setPreviewWindow(binder, window);
}

void  CameraScanner::setPreviewCallbackFlag(int callback_flag){
    mPreviewCallbackFlag = callback_flag;
    if (mPreviewCallbackFlag & CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK) {
    	ALOGE("CameraScanner1:%s",__FUNCTION__);
        enableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    } else {
    	ALOGE("CameraScanner2:%s",__FUNCTION__);
        disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    }
}
status_t CameraScanner::startPreview(){
	ALOGE("CameraScanner:%s",__FUNCTION__);
	Mutex::Autolock lock(mLock);
    return startPreviewMode();
}
void CameraScanner::stopPreview(){
	ALOGE("stopPreview:%s",__FUNCTION__);
    Mutex::Autolock lock(mLock);

    disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
    mHardware->stopPreview();
}

status_t CameraScanner::startPreviewMode() {
    // if preview has been enabled, nothing needs to be done
    ALOGE("startPreviewMode:%s",__FUNCTION__);
    if (mHardware->previewEnabled()) {
        return NO_ERROR;
    }
    if (mPreviewWindow != 0) {
        native_window_set_scaling_mode(mPreviewWindow.get(),
                NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        native_window_set_buffers_transform(mPreviewWindow.get(),
                mOrientation);
    }
    mHardware->setPreviewWindow(NULL);
    status_t result = mHardware->startPreview();
    ALOGE("startPreviewMode:-");
    return result;
}

bool CameraScanner::previewEnabled(){
	Mutex::Autolock lock(mLock);
	return mHardware->previewEnabled();
}
status_t CameraScanner::storeMetaDataInBuffers(bool enabled){
	return OK;
}
status_t CameraScanner::startRecording(){
	return OK;
}
void CameraScanner::stopRecording(){

}
status_t CameraScanner::autoFocus(){
	return OK;
}
status_t CameraScanner::cancelAutoFocus(){
	return OK;
}
status_t CameraScanner::takePicture(int msgType){
	return OK;
}
status_t CameraScanner::setParameters(const String8& params){
    CameraParameters p(params);
    return mHardware->setParameters(p);
}
String8 CameraScanner::getParameters(){
    Mutex::Autolock lock(mLock);
    //if (checkPidAndHardware() != NO_ERROR) return String8();

    String8 params(mHardware->getParameters().flatten());
    return params;
}
status_t CameraScanner::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2){
	return mHardware->sendCommand(cmd, arg1, arg2);
}

void CameraScanner::enableMsgType(int32_t msgType) {
    android_atomic_or(msgType, &mMsgEnabled);
    mHardware->enableMsgType(msgType);
}

void CameraScanner::disableMsgType(int32_t msgType) {
    android_atomic_and(~msgType, &mMsgEnabled);
    mHardware->disableMsgType(msgType);
}
// these are static callback functions
static void CameraScanner::notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2, void* user){
	ALOGE("CameraScanner:%s",__FUNCTION__);
	if (sCameraScanner != NULL) {
	    sp<CameraListener> listener;
	    {
	        listener = sCameraScanner->mListener;
	    }
	    if (listener != NULL) {
	    	listener->notify(msgType, ext1, ext2);
	    }
	}
}
static void CameraScanner::dataCallback(int32_t msgType, const sp<IMemory>& dataPtr,
        camera_frame_metadata_t *metadata, void* user){
	ALOGE("CameraScanner:%s",__FUNCTION__);
	if (sCameraScanner != NULL) {
	    sp<CameraListener> listener;
	    {
	        listener = sCameraScanner->mListener;
	    }
	    if (listener != NULL) {
	    	listener->postData(msgType, dataPtr, metadata);
	    }
	}
}
static void CameraScanner::dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr, void* user){
	ALOGE("CameraScanner:%s",__FUNCTION__);
	if (sCameraScanner != NULL) {
	    sp<CameraListener> listener;
	    {
	        listener = sCameraScanner->mListener;
	    }
	    if (listener != NULL) {
	    	listener->postDataTimestamp(timestamp, msgType, dataPtr);
	    }
	}
}

