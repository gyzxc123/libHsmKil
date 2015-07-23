/*======================================================================

	UNLESS OTHERWISE AGREED TO IN A SIGNED WRITING BY HONEYWELL INTERNATIONAL INC
	(HONEYWELL) AND THE USER OF THIS CODE, THIS CODE AND INFORMATION IS PROVIDED
	"AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
	FOR A PARTICULAR PURPOSE.

	COPYRIGHT (C) 2008 HONEYWELL INTERNATIONAL INC.

	THIS SOFTWARE IS PROTECTED BY COPYRIGHT LAWS OF THE UNITED STATES OF
	AMERICA AND OF FOREIGN COUNTRIES. THIS SOFTWARE IS FURNISHED UNDER A
	LICENSE AND/OR A NONDISCLOSURE AGREEMENT AND MAY BE USED IN ACCORDANCE
	WITH THE TERMS OF THOSE AGREEMENTS. UNAUTHORIZED REPRODUCTION,  DUPLICATION
	OR DISTRIBUTION OF THIS SOFTWARE, OR ANY PORTION OF IT  WILL BE PROSECUTED
	TO THE MAXIMUM EXTENT POSSIBLE UNDER THE LAW.

======================================================================*/

#pragma once

// DecoderJNI.h : header file for Decoder API functions related to Android
#include "types.h"
#include "Decoder.h"
#include <jni.h>

//======================================================================================
//  Decoder Engine API Functions for Android
//======================================================================================

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

Result_t decWaitForDecodeJNI (DWORD dwTimeout, DecodeMsg_t *pRes, JNIEnv* env, jobject obj);
Result_t decWaitMultipleDecodeJNI(DWORD dwTimeout, JNIEnv* env, jobject obj);

#ifdef __cplusplus
}
#endif  /* __cplusplus */



