# Hey Emacs, this is a -*- makefile -*-
#
# Makefile for generating a hardware layer lib for Android
# by Dieter Fauth, Honeywell Scanning & Mobility
#
#########################################################################################
# Platform adjustments section
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

TARGET_PLATFORM = MTK

PROJECT_ROOT = .
#include platform.mk
#include common_vars.mk

# NDK could call us several times, we better clean our variables
CONFIGFLAGS :=
HWLAYER := 

#########################################################################################
# Options section
# Control the debug options
# You can turn debugging on by adding "DEBUG=yes" to the make comand line.
# Change the next lines to enable debug messsages and HWTRACEx
DEBUG ?= yes
HWTRACE ?= no
USE_TIMEDEBUG ?= no
# Adds some image debugging code to hardware layer. (This code is not part of the HWL distribution, so you normally have "no" as value)
USE_IMAGEDEBUG ?= no

ifeq ($(DEBUG),yes)
CONFIGFLAGS += DEBUG=1
CONFIGFLAGS += SHOWERRORS=1
CONFIGFLAGS += CAMERA_DEVICE_DEBUG_ENABLED
CONFIGFLAGS += GPIO_DEBUG_ENABLED
CONFIGFLAGS += I2C_DEBUG_ENABLED
CONFIGFLAGS += HWLAYER_SKEL_DEBUG_ENABLED
CONFIGFLAGS += QUEUE_BUFFER_DEBUG_ENABLED
#CONFIGFLAGS += USING_PRINTF			# output debug messages with printf instead of logcat
else
CONFIGFLAGS += NDEBUG NO_DEBUG=1
endif

# Using a GPIO line to debug with the scope.
ifeq ($(USE_TIMEDEBUG),yes)
CONFIGFLAGS += USE_TIMEDEBUG
endif

# Using a GPIO line to debug with the scope.
ifeq ($(HWTRACE),yes)
HWLAYER += DebugHelpers_hw.cpp
CONFIGFLAGS += USE_HWTRACE
endif

# A way to debug image problems in the hardware layer.
ifeq ($(USE_IMAGEDEBUG),yes)
HWLAYER += $(PROJECT_ROOT)/TestCode/ImageDebugLib/ImageDebug.cpp
CONFIGFLAGS += USE_IMAGEDEBUG
endif

CONFIGFLAGS+= ARM=1
CONFIGFLAGS+= CONFIG_HWLAPI=1
CONFIGFLAGS+= ASSERT=assert

CONFIGFLAGS+= SVN_Revision="TODO"

#########################################################################################
# Hardware platform section
# Defines features specific to hardware platform

HWPLATFORM ?= GW552

ifeq ($(HWPLATFORM), D7800)
else ifeq ($(HWPLATFORM), PM40)
else ifeq ($(HWPLATFORM), GW552)
CAMERA_ID = 1
else
CAMERA_ID = 1
endif

# Configure features:
# Names for devices
CONFIGFLAGS+= HWPLATFORM_$(HWPLATFORM)

ifdef CAMERA_ID
CONFIGFLAGS += ANDROID_CAMERA_ID=$(CAMERA_ID)
endif

#########################################################################################
# Target section
# The name for the generated lib
# Android does not use the linux way of versioning dynamic libraries
PACKETNAME = libHsmKil
DESTINATION_DIR = $(PROJECT_ROOT)/libs/android/$(TARGET_ARCH_ABI)

#########################################################################################
# File section
# Objects built from CPP source files
HWLAYER += HWLayerN5600.cpp
HWLAYER += HWLayerIT5000.cpp
HWLAYER += HWLayer_Skel.cpp
HWLAYER += gpio.cpp
HWLAYER += i2c.cpp
HWLAYER += capture.cpp
HWLAYER += queue.cpp # queue class for buffer management
HWLAYER += DebugHelpers.cpp
HWLAYER += CameraScanner.cpp
#HWLAYER += camera_device.cpp
ifeq ($(TARGET_PLATFORM),MTK)
HWLAYER += camera_device_acam.cpp
else
HWLAYER += camera_device.cpp
endif
$(warning $(TARGET_PLATFORM))
#########################################################################################
# Places section
# List any extra directories to look for include files here.
EXTRAINCDIRS := .
EXTRAINCDIRS += $(TOP)/system/core/include
EXTRAINCDIRS += $(TOP)/system/core/include/cutils
EXTRAINCDIRS += $(LOCAL_PATH)/include
EXTRAINCDIRS += $(LOCAL_PATH)/android
EXTRAINCDIRS += $(TOP)/mediatek/hardware/camera/inc

#########################################################################################
# Libraries section


#########################################################################################
# Compiler/Linker/tools control section
# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS :=
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += $(LIBSEARCH)
CFLAGS += -Wall
CFLAGS += -fpermissive
#CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Werror=non-virtual-dtor
CFLAGS += -Wabi
CFLAGS += -Wpointer-arith -Wswitch

#CFLAGS +=  -H	      # Print the name of each header used
CFLAGS += -ffunction-sections
CFLAGS += -fno-ident
CFLAGS += -fno-delete-null-pointer-checks # avoid optimizations for pointers (would remove if(ptr==NULL) errorfunction)

#CFLAGS += -fvisibility=hidden

CFLAGS += -Wmissing-include-dirs
CFLAGS += -fno-builtin

#CFLAGS += -Wfatal-errors

DISABLED_WARNINGS += -Wno-sign-compare
DISABLED_WARNINGS += -Wno-unused-variable
DISABLED_WARNINGS += -Wno-unused-parameter
DISABLED_WARNINGS += -Wno-unused-value
DISABLED_WARNINGS += -Wno-missing-braces
DISABLED_WARNINGS += -Wno-comment
CFLAGS += $(DISABLED_WARNINGS)

#########################################################################################
# Android section

LOCAL_MODULE		:= $(PACKETNAME)
LOCAL_SRC_FILES		:= $(HWLAYER)
LOCAL_C_INCLUDES	:= $(EXTRAINCDIRS)

LOCAL_CFLAGS		:= $(CFLAGS) $(addprefix -D, $(CONFIGFLAGS))
LOCAL_CPPFLAGS 		+= $(CPPFLAGS)
LOCAL_ARM_MODE		:= arm

LOCAL_LDLIBS		:= -llog

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libhardware

#LOCAL_STATIC_LIBRARIES = libcamdrv
#LOCAL_SHARED_LIBRARIES =

# The next two are ignored by the NDK r6. Use default.properties file instead.
#TARGET_PLATFORM := android-9
#APP_ABI := armeabi
LOCAL_MODULE_TAGS := optional eng
LOCAL_PRELINK_MODULE := false

# Name of the APK to build
LOCAL_PACKAGE_NAME := $(PACKETNAME)

include $(BUILD_SHARED_LIBRARY)


ifdef DESTINATION_DIR 
$(LOCAL_INSTALLED): EXPORT_SRC := $(LOCAL_BUILT_MODULE)
$(LOCAL_INSTALLED): EXPORT_NAME := $(notdir $(LOCAL_BUILT_MODULE))
$(LOCAL_INSTALLED): export

.PHONY: export
export: $(EXPORT_SRC)
	@ echo "Install        : $(EXPORT_NAME) => $(DESTINATION_DIR)/$(EXPORT_NAME)"
	$(hide) install -p $(EXPORT_SRC) $(DESTINATION_DIR)/$(EXPORT_NAME)
endif


ifdef APK
# Tell it to build an APK
include $(BUILD_PACKAGE)
endif

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS   := optional eng
LOCAL_MODULE        := libHSMDecoderAPI
LOCAL_PREBUILT_LIBS := libs/libHSMDecoderAPI.so
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS   := optional eng
LOCAL_MODULE        := libHHPScanInterface
LOCAL_PREBUILT_LIBS := libs/libHHPScanInterface.so
include $(BUILD_MULTI_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))
