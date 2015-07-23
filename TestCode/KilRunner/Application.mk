# Hey Emacs, this is a -*- makefile -*-
# by Dieter Fauth, Honeywell Scanning & Mobility
#
#NDK_DEBUG_IMPORTS=1

APP_ABI := armeabi # armeabi-v7a x86 mips
APP_PLATFORM := android-10
#TARGET_PLATFORM := android-9

APP_STL := stlport_static    #--> static STLport library
#APP_STL := stlport_shared    #--> shared STLport library
#APP_STL := system            #--> default C++ runtime library

ifeq ($(DEBUG),yes)
APP_DEBUGGABLE := true
APP_OPTIM := debug
endif

