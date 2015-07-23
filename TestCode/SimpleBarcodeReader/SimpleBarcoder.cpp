/*===================================================================
  Example source for a very simple comand line barcode reader.

   $Source: Linux/SimpleBarcodeReader/SimpleBarcoder.cpp $
   $Author: Fauth,Dieter (E411776) $
   $Date: 2012/02/07 11:17:07EST $
   $Revision: 1.12 $

   Copyright I Honeywell, Inc. 2008
===================================================================*/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "Os.h"
#include <Decoder.h>
#if 0
#include "SimpleImage.h"
#endif


using namespace std;

#define UINT unsigned int
#define FALSE	0
#define TRUE	1

#define ENGINE_API_RESPONSE_LEN 200

///////////////////////////////////////////////////////////////////////////////////////////
// debug helpers
// Set to 1 if you want to store the last image to a file
#define AUTO_STORE_LAST_IMAGE	0

///////////////////////////////////////////////////////////////////////////////////////////
void WriteLastImageToFile();

///////////////////////////////////////////////////////////////////////////////////////////
void ShowHSMError(Result_t errnum)
{
	static CHAR pErrorText[ENGINE_API_RESPONSE_LEN];
	if(errnum!=RESULT_SUCCESS)
	{
		decGetErrorMessage(pErrorText, errnum);
		printf("Error: %s\n", pErrorText);
	}
}

// Shows the MKS version strings (note: hardware layer is not part of the official version API)
void ShowVersions()
{
	Result_t RetVal=RESULT_INITIALIZE;
	static CHAR Response[100];

	puts("Versions:");
	//Result_t decGetAPIRevision(CHAR* pszRev);
	RetVal = decGetAPIRevision(Response);

	ShowHSMError(RetVal);
	if(RetVal == RESULT_SUCCESS)
	{
		printf("%-14s: %s\n", "API", Response);
	}

	//Result_t decGetScanDriverRevision(CHAR* pszRev);
	RetVal =  decGetScanDriverRevision(Response);
	ShowHSMError(RetVal);
	if(RetVal == RESULT_SUCCESS)
	{
		printf("%-14s: %s\n", "Scandriver", Response);
	}

	//Result_t decGetDecoderRevision(CHAR* pszRev);
	RetVal =  decGetDecoderRevision(Response);
	ShowHSMError(RetVal);
	if(RetVal == RESULT_SUCCESS)
	{
		printf("%-14s: %s", "Decoder", Response);
	}

#if 0 // not supported 
	int PsocMajor=0;
	int PsocMinor=0;
	//Result_t decGetPSOCMajorRev(int *pMajorRev);
	RetVal =  decGetPSOCMajorRev(&PsocMajor);

	//Result_t decGetPSOCMinorRev(int *pMinorRev);
	RetVal =  decGetPSOCMinorRev(&PsocMinor);
	printf("%-14s: %i.%i\n", "PSOC", PsocMajor,PsocMinor);

	//Result_t decGetEngineSerialNumber(CHAR* pszRev);
	RetVal =  decGetEngineSerialNumber(Response);
	ShowHSMError(RetVal);
	if(RetVal == RESULT_SUCCESS)
	{
		printf("%-14s: %s\n", "Serial number", Response);
	}
#endif
}

/* eml: quick fix - do not need
static BOOL Symbologies[MAX_SYMBOLOGIES] =
{
		0
};
*/
// example setup
void SetupDecoder(void)
{
	/*
	Result_t RetVal=RESULT_INITIALIZE;

//	RetVal = oemSetExposureMode(EXP_MODE_ONCHIP);
//	ShowHSMError(RetVal);

 	ExposureSettings_t Exposure;
	RetVal = oemGetExposureSettings(&Exposure);
	ShowHSMError(RetVal);
	Exposure.ImageMustConform = FALSE;
//	Exposure.ImageMustConform = TRUE;
	RetVal = oemSetExposureSettings(&Exposure);
	ShowHSMError(RetVal);


//	oemGetSearchTimeLimitEx(SETUP_TYPE_CURRENT, WORD nLevel, WORD *pnLimit)
//	ShowHSMError(RetVal);

//	RetVal = oemSetScanningLightsMode(SCAN_ILLUM_AIMER_ON);
//	ShowHSMError(RetVal);


//	RetVal = oemSetScanningLightsMode(SCAN_ILLUM_AIMER_OFF);
//	RetVal = oemSetScanningLightsMode(SCAN_ILLUM_ONLY_ON);
//	RetVal = oemSetScanningLightsMode(SCAN_AIMER_ONLY_ON);
	RetVal = oemSetScanningLightsMode(SCAN_ILLUM_AIMER_ON);
	ShowHSMError(RetVal);

	RetVal = oemEnableSymbologyAll();
	ShowHSMError(RetVal);
	*/

/* Shows how to disable all sybologies and enables only one.
	RetVal = oemDisableSymbologyAll();
	ShowHSMError(RetVal);
	Symbologies[SYM_PDF417] = TRUE;
	RetVal =  oemEnableSymbology( Symbologies );
	ShowHSMError(RetVal);
*/

}


// The barcode scanner
void CaptureAndDecode(void)
{
	//SetupDecoder(); // eml: use default setup for now
	unsigned int StartTime = GetTickCount();
	const DWORD dwTime=3000*2;
	const DWORD SIZE=6000;
	static BYTE pchMessage[SIZE+10];

	DWORD DecodeTime=0;
	Result_t RetVal=RESULT_ERR_NODECODE;

	DecodeMsg_t myDecodeMsg;

	myDecodeMsg.pchMessage = pchMessage;

	RetVal = decWaitForDecode(dwTime, &myDecodeMsg, NULL);
	if(RetVal == RESULT_SUCCESS)
	{
		unsigned int EndTime = GetTickCount();
		unsigned int TotalTime = EndTime-StartTime;

		printf("----------------------------------\n");
		printf("Decoded=%i bytes, AIMid=%c, HHPid=%c, SymMod=%c\n", myDecodeMsg.nLength, (int)myDecodeMsg.chCodeID, (int)myDecodeMsg.chSymLetter, myDecodeMsg.chSymModifier);
		RetVal = decGetLastDecodeTime(&DecodeTime);
		ShowHSMError(RetVal);
		printf("DecodeTime=%i ms, Total=%i ms\n", DecodeTime, TotalTime);

		printf("Decoded Message: ");
		puts((const char*)myDecodeMsg.pchMessage);
		printf("\n----------------------------------\n");
	}
	else
	{
		puts("decWaitForDecode failed\n");
		ShowHSMError(RetVal);
	}
}

void CaptureAndDecode_UI(void)
{
	puts("CaptureAndDecode\n");
	CaptureAndDecode();
}

void CaptureAndDecodeContinuous()
{
	while(!kbhit())
	{
		CaptureAndDecode();
	}
}


void ShowHelp(char Rx)
{
	if(Rx!=0)
		printf("Unrecognized command: %c\n", Rx);
	puts("Commands:");
	puts("  r   Read the next barcode");
	puts("  R   Read continuously until a character from the debug port is received");
	puts("  v V Show the decoder version");
	puts("  w   Write last image to file");
	puts("  q Q Exit");
	puts("  x X Exit");
}

void SimpleBarcodeReader(void)
{
	puts("==========SimpleBarcodeReader===============");
	Result_t RetVal=RESULT_ERR_NODECODE;

	RetVal = decConnect();
	if(RetVal == RESULT_SUCCESS)
	{
 		puts("decConnect OK");
	}
	else
	{
 		printf("decConnect failed=%i\n", RetVal);
		return;
	}

	puts("Commands: r R v q x (i I l L) (other characters show the help)\n$ ");
	ShowHelp(0);
	while(1)
	{
		printf("$ ");
		int Rx=getchar();
		switch(Rx)
		{
			//Read
			case 'r':
				CaptureAndDecode_UI();
				break;
			case 'R':
				CaptureAndDecodeContinuous();
				break;
			case 'v':
			case 'V':
				ShowVersions();
				break;
			case 'w':
				WriteLastImageToFile();
				break;
			case 'q':
			case 'Q':
			case 'x':
			case 'X':
				decDisconnect();
				return;
				break;
			case '\r':
			case '\n':
				puts("");
				break;
			default:
				ShowHelp(Rx);
				break;
		}
	}
}

int main( int argc, char ** argv )
{
	CTerminal NonCanonical;

	SimpleBarcodeReader();
	puts("Terminated");

	return 0;
}

bool WriteAsPgmImageToFile(const char *pName, unsigned char* pImage, unsigned long Size, int Columns, int Rows)
{
	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	snprintf(Name, MaxNameLen, "/sdcard/%s.pgm", pName);
	printf("Writing %s\n", Name);

	bool bStatus=false;
	FILE * hFile;
	hFile = fopen( Name, "w-");
	if (hFile != NULL)
	{
		fprintf(hFile, "P5\n%u %u\n255\n", Columns, Rows);
		unsigned int written = fwrite(pImage, sizeof(unsigned char), Size, hFile);
		if(written!=Size)
		{
			printf("WriteImageToFile failed!!\n");
		}
		fclose(hFile);
		bStatus=true;
	}
	return bStatus;
}

static int NameCounter=0;

void WriteLastImageToFile()
{
	WORD Rows=0;
	WORD Columns=0;
	DWORD dwSize=0;
	ImageAttributes_t ImageAttributes;
	ImageAttributes.Size=sizeof(ImageAttributes_t);

	const int MaxNameLen=100;
	char Name[MaxNameLen+1];

	Result_t RetVal = decGetLastImageSize(&Columns, &Rows, &dwSize);
	if(RetVal == RESULT_SUCCESS)
	{
		unsigned char *pBmpBuffer = new unsigned char[dwSize+100];
		RetVal = decGetLastImage (pBmpBuffer, &dwSize, &ImageAttributes);
		if(RetVal == RESULT_SUCCESS)
		{
			snprintf(Name, MaxNameLen, "img%02d", NameCounter++);
			if( WriteAsPgmImageToFile(Name, pBmpBuffer, dwSize, Columns, Rows) )
			{
			}
		}
		else
		{
			printf("Failed to get last image, ERR=%d\n", RetVal);
		}

		delete [] pBmpBuffer;
	}
	else
	{
		printf("Failed to get last image size, ERR=%d\n", RetVal);
	}
}
