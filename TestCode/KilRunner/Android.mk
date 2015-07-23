# Hey Emacs, this is a -*- makefile -*-
#
# Makefile for generating a hardware layer lib for Android
# by Dieter Fauth, Honeywell Scanning & Mobility
#
#########################################################################################
# Platform adjustments section
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

PROJECT_ROOT := $(LOCAL_PATH)/..
#include $(PROJECT_ROOT)/HardwareLayer/platform.mk
#include $(PROJECT_ROOT)/HardwareLayer/common_vars.mk

# NDK could call us several times, we better clean our variables

#########################################################################################
# Options section
CONFIGFLAGS :=

# Control the debug options
# You can turn debugging on by adding "DEBUG=yes" to the make comand line.
DEBUG ?= yes

ifeq ($(DEBUG),yes)
CONFIGFLAGS += DEBUG=1 MSCGEN=0
CONFIGFLAGS += SHOWERRORS=1
#CONFIGFLAGS += HAS_FRAMEBUFFER=1
CONFIGFLAGS += USING_PRINTF			# output debug messages with printf instead of logcat
else
CONFIGFLAGS += NDEBUG NO_DEBUG=1
endif

USE_IMAGEDEBUG = yes
# A way to debug image problems in the hardware layer.
ifeq ($(USE_IMAGEDEBUG),yes)
CONFIGFLAGS += USE_IMAGEDEBUG
endif

# Macros to control conditional compile
CONFIGFLAGS+= ARM=1
CONFIGFLAGS+= CONFIG_HWLAPI=1
CONFIGFLAGS+= ASSERT=assert
CONFIGFLAGS+= $(NODEBUG)

CONFIGFLAGS+= SVN_Revision=\"TODO\"

#########################################################################################
# Target section
# The name for the generated lib
# Android does not use the linux way of versioning dynamic libraries
PACKETNAME = KilRunner
TARGET = $(DESTINATION_BIN)/$(PACKETNAME)

#########################################################################################
# File section
# Objects built from CPP source files
KILRUNNER =
KILRUNNER += KilRunner.cpp
KILRUNNER += ImageTester.cpp
KILRUNNER += ImageTesterBase.cpp
KILRUNNER += Gen5ImageTester.cpp
KILRUNNER += Gen6ImageTester.cpp
KILRUNNER += PowerTester.cpp
#KILRUNNER += UserSpaceI2c.cpp

#########################################################################################
# Places section
# List any extra directories to look for include files here.
EXTRAINCDIRS := .
EXTRAINCDIRS += $(PROJECT_ROOT)/include
EXTRAINCDIRS += $(TOP)/prebuilts/ndk/current/sources/cxx-stl/stlport/stlport
#EXTRAINCDIRS += $(LOCAL_PATH)/../../libHsmKil
EXTRAINCDIRS += $(PROJECT_ROOT)/..

#########################################################################################
# Libraries section
IMPORT_LIBS := yes
USED_LIBS :=

#ifeq ($(HY),yes)
# linking ImageDebug lib (is there an easier way? This looks quite complicated.
# relative to jni dir
DEBUG_LIBDIR = $(TARGET_OUT)/lib
DEBUG_LIB = libImageDebug

ifeq ($(IMPORT_LIBS), yes)
LOCAL_SRC_FILES := $(DEBUG_LIBDIR)/$(DEBUG_LIB).so
LOCAL_MODULE    := $(DEBUG_LIB)
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)
USED_LIBS += $(DEBUG_LIB)
else
USED_LIBS += -L$(DEBUG_LIBDIR) -l$(DEBUG_LIB)
endif


# linking hwl/kil lib (is there an easier way? This looks quite complicated.
# relative to jni dir
KIL_LIBDIR = $(TARGET_OUT)/lib
KIL = libHsmKil

ifeq ($(IMPORT_LIBS), yes)
LOCAL_SRC_FILES := $(KIL_LIBDIR)/$(KIL).so
LOCAL_MODULE    := $(KIL)
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)
USED_LIBS += $(KIL)
else
USED_LIBS += -L$(KIL_LIBDIR) -l$(KIL)
endif
#endif

#########################################################################################
# Compiler/Linker/tools control section
# Compiler flags.
CFLAGS :=

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
LOCAL_SRC_FILES		:= $(KILRUNNER)
LOCAL_C_INCLUDES	:= $(EXTRAINCDIRS)

LOCAL_CFLAGS		:= $(CFLAGS) $(addprefix -D, $(CONFIGFLAGS))
LOCAL_CPPFLAGS 		+= $(CPPFLAGS)
LOCAL_ARM_MODE		:= arm

LOCAL_LDFLAGS		:= -Wl,-rpath,.
LOCAL_LDLIBS		+= -llog

ifeq ($(IMPORT_LIBS), yes)
LOCAL_SHARED_LIBRARIES = $(USED_LIBS)
LOCAL_SHARED_LIBRARIES += libImageDebug
LOCAL_SHARED_LIBRARIES += libHsmKil
LOCAL_SHARED_LIBRARIES += libstlport
else
LOCAL_LDLIBS		+= $(USED_LIBS)
endif

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

# Name of the APK to build
LOCAL_PACKAGE_NAME := $(PACKETNAME)

include $(BUILD_EXECUTABLE)

ifdef APK
# Tell it to build an APK
include $(BUILD_PACKAGE)

include $(PROJECT_ROOT)/HardwareLayer/common_rules.mk
endif



