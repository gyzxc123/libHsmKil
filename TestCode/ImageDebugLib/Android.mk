# Hey Emacs, this is a -*- makefile -*-
#
# Makefile for generating a hardware layer lib for Android
# by Dieter Fauth, Honeywell Scanning & Mobility
#
#########################################################################################
# Platform adjustments section
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

PROJECT_ROOT := $(LOCAL_PATH)
#include $(PROJECT_ROOT)/HardwareLayer/platform.mk
#include $(PROJECT_ROOT)/HardwareLayer/common_vars.mk

#########################################################################################
# Options section
# Control the debug options
# You can turn debugging on by adding "DEBUG=yes" to the make comand line.
DEBUG=yes
ifeq ($(DEBUG),yes)
CONFIGFLAGS += DEBUG=1 MSCGEN=1
CONFIGFLAGS += USING_PRINTF			# output debug messages with printf instead of logcat
USE_IMAGEDEBUG = yes
else
CONFIGFLAGS += NDEBUG NO_DEBUG=1
endif

# A way to debug image problems in the hardware layer.
ifeq ($(USE_IMAGEDEBUG),yes)
CONFIGFLAGS += USE_IMAGEDEBUG
endif

# Macros to control conditional compile
CONFIGFLAGS+= ARM=1
CONFIGFLAGS+= CONFIG_HWLAPI=1
CONFIGFLAGS+= ASSERT=assert
CONFIGFLAGS+= $(NODEBUG)

#########################################################################################
# Target section
# The name for the generated lib
# Android does not use the linux way of versioning dynamic libraries
PACKETNAME = libImageDebug
TARGET = $(DESTINATION_LIB)/$(PACKETNAME).so

#########################################################################################
# File section
# Objects built from CPP source files
DEBUGHELPERS =
DEBUGHELPERS += ImageDebug.cpp
DEBUGHELPERS += ImgHelper.cpp
DEBUGHELPERS += DebugHelpers.cpp
DEBUGHELPERS += mscgen.cpp
DEBUGHELPERS += Os.cpp

#########################################################################################
# Places section
# List any extra directories to look for include files here.
EXTRAINCDIRS := .
#EXTRAINCDIRS += $(TOP)/vendor/zltd/CameraScanner/Honeywell/TestCode/include
EXTRAINCDIRS += $(PROJECT_ROOT)/../include
#########################################################################################
# Libraries section
USED_LIBS :=

#########################################################################################
# Compiler/Linker/tools control section
# Compiler flags.

# Hide all internal functions
CFLAGS += -fvisibility=hidden

# Don't recognize built-in functions that do not begin with __builtin_ as prefix.
# FIXME: can this go?
CFLAGS += -fno-builtin

# no exceptions (need to double check, it is here historically)
CPPFLAGS += -fno-exceptions

DISABLED_WARNINGS += -Wno-sign-compare
DISABLED_WARNINGS += -Wno-unused-variable
DISABLED_WARNINGS += -Wno-unused-parameter
DISABLED_WARNINGS += -Wno-unused-value
#DISABLED_WARNINGS += -Wno-comment

# Only use if you really need it
#CFLAGS += -Wno-format
#CFLAGS += -Wno-packed
#CFLAGS += -fno-strict-aliasing

# Some optional but usefull flags
# Be more verbose
#CFLAGS += -v
# Print the name of each header used (good for hunting issues with include files)
#CFLAGS += -H

#########################################################################################
# Android section

LOCAL_MODULE		:= $(PACKETNAME)
LOCAL_SRC_FILES		:= $(DEBUGHELPERS)
LOCAL_C_INCLUDES	:= $(EXTRAINCDIRS)

LOCAL_CFLAGS		:= $(CFLAGS) $(addprefix -D, $(CONFIGFLAGS))
LOCAL_CPPFLAGS 		+= $(CPPFLAGS)
LOCAL_ARM_MODE		:= arm

LOCAL_LDFLAGS		:=
LOCAL_LDLIBS		:= -llog 

LOCAL_SHARED_LIBRARIES = $(USED_LIBS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

# Name of the APK to build
LOCAL_PACKAGE_NAME := $(PACKETNAME)

include $(BUILD_SHARED_LIBRARY)

ifdef APK
# Tell it to build an APK
include $(BUILD_PACKAGE)

include $(PROJECT_ROOT)/HardwareLayer/common_rules.mk
endif

