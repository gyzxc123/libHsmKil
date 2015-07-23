# Hey Emacs, this is a -*- makefile -*-
#
# Makefile for generating the simple example for Android
# by Honeywell Scanning & Mobility
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
PROJECT_ROOT := $(LOCAL_PATH)/..

SBRSOURCE := SimpleBarcoder.cpp
SBRSOURCE += Os.cpp

EXTRAINCDIRS := $(PROJECT_ROOT)/include

LOCAL_MODULE		:= SimpleBarcodeReader
LOCAL_SRC_FILES		:= $(SBRSOURCE)
LOCAL_C_INCLUDES	:= $(EXTRAINCDIRS)

LOCAL_SHARED_LIBRARIES += libHSMDecoderAPI
LOCAL_SHARED_LIBRARIES += libHHPScanInterface
LOCAL_SHARED_LIBRARIES += libHsmKil

LOCAL_LDFLAGS		+= -Wl,-rpath,.
LOCAL_ARM_MODE		:= arm

include $(BUILD_EXECUTABLE)
