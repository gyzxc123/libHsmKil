/*===================================================================
  Debug code. Use this to mess around and keep SimpleBarcodeReader clean.

   $Source: Linux/Android/TestCode/DebugRunner/MultiDecode.cpp $
   $Author: Fauth,Dieter (E411776) $
   $Date: 2012/06/22 11:17:05EDT $
   $Revision: 1.1 $

   Copyright Honeywell, Inc. 2008
===================================================================*/


#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

#include "BarcodeDecoder.h"
#include "Os.h"
#include "ImageTester.h"
#include "ImageDebug.h"


using namespace std;

#define UINT unsigned int
#define FALSE	0
#define TRUE	1

BOOL ShowContents(DecodeMsgRaw_t* pMsg)
{
	puts("----------------------------------\n");
	printf("Decoded=%i bytes, CodeID=%c, AimID=%c, AimMod=%c, ",
			pMsg->nLength, (int)pMsg->chCodeID, (int)pMsg->chSymLetter, (int)pMsg->chSymModifier);

	pMsg->pchMessage[pMsg->nLength] = 0;
	puts((const char*)pMsg->pchMessage);
	puts("\n----------------------------------\n");
	return TRUE;
}


BOOL DecMultiReadCallBack(DecodeMsgRaw_t* decMsg)
{
	puts("DecMultiReadCallBack");
	BOOL bResult = FALSE;
	if (decMsg == NULL)
	{
		printf("ERROR => 'decMsg' == NULL!!!\n");
	}
	else if (decMsg->pchMessage == NULL)
	{
		printf("ERROR => BC value MISSING!!!\n");
	}
	else
	{
		bResult = ShowContents(decMsg);
	}

	return bResult;
}

BOOL DecKeepGoingCallBack(void)
{
/*
 	if(m_pInstance != NULL)
	{
		if(sem_trywait(&m_pInstance->m_semDecThread_DecStop) != 0)
		{
			return TRUE;
		}
	}
*/
	printf("DecKeepGoingCallBack\n");
	return TRUE;
}

void SetupDecoder(void);

void CaptureAndDecodeMultiple()
{
	const DWORD dwTime=3000/2;
	SetupDecoder();
// Decoding start....
	Result_t decResult = oemWaitMultipleDecodeRaw(dwTime, DecMultiReadCallBack, DecKeepGoingCallBack);
}
