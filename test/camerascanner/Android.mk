LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#########################################################################################
# File section
# Objects built from CPP source files
EXTRA_CPPS += ../../CameraScanner.cpp
EXTRA_CPPS += scanner.cpp

#########################################################################################
# Places section
# List any extra directories to look for include files here.
EXTRA_INCDIRS := .
EXTRA_INCDIRS += $(TOP)/system/core/include
EXTRA_INCDIRS += $(TOP)/system/core/include/cutils
EXTRA_INCDIRS += $(LOCAL_PATH)/../../include
EXTRA_INCDIRS += $(LOCAL_PATH)/../..

# Android section

LOCAL_MODULE		:= camerascanner
LOCAL_SRC_FILES		:= $(EXTRA_CPPS)

LOCAL_C_INCLUDES	:= $(EXTRA_INCDIRS)

LOCAL_CFLAGS		:= -fpermissive

LOCAL_LDLIBS		:= -llog

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libhardware

#LOCAL_STATIC_LIBRARIES = libcamdrv
#LOCAL_SHARED_LIBRARIES =

LOCAL_MODULE_TAGS := optional eng
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)
