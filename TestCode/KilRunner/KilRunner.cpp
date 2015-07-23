/*===================================================================
  Debug code. Use this to mess around and keep SimpleBarcodeReader clean.

   $Source: Linux/Android/TestCode/KilRunner/KilRunner.cpp $
   $Author: Fauth,Dieter (E411776) $
   $Date: 2012/06/22 11:18:22EDT $
   $Revision: 1.8 $

   Copyright I Honeywell, Inc. 2008
===================================================================*/


#include "commoninclude.h"
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

#include <Os.h>
#include <IHwl.h>

#include "Gen5ImageTester.h" // 	until we distinguish the imager automatically
#include "Gen6ImageTester.h" // 	until we distinguish the imager automatically

using namespace std;

extern "C" void GetHWLayerRevision(char * pszRev, size_t BufferSize);

#define NEW_AIMER_AND_ILLUMINATION_TEST 1 // defined to test new aimer and illumination tests

//////////////////////////////////////////////////////////////////////////////////////////
void ShowHSMError(Result_t errnum)
{
//	static char pErrorText[ENGINE_API_RESPONSE_LEN];
	if(errnum!=RESULT_SUCCESS)
	{
//		oemGetErrorMessage(pErrorText, errnum);
//		puts(pErrorText);
		printf("ERROR=%i\n", errnum);
	}
}

static IHwl* gs_Hwl=NULL;

void InitKil()
{
	gs_Hwl = InitHWLayer(NULL);
	ASSERT(gs_Hwl!=NULL);

#if USE_SESSION
	if(!gs_Hwl->OpenSession())
	{
		puts("OpenSession failed");
	}
#endif
	if (!gs_Hwl->ImagerPowerOn())
	{
		puts("ERROR: ImagerPowerOn failed.");
	}
	if (!gs_Hwl->ImagerPowerUp())
	{
		puts("ERROR: ImagerPowerUp failed.");
	}
}

void TerminateKil()
{
	ASSERT(gs_Hwl!=NULL);
	gs_Hwl->ImagerPowerDown();
	gs_Hwl->ImagerPowerOff();
#if USE_SESSION
	gs_Hwl->CloseSession();
#endif
	DeinitHWLayer(gs_Hwl);
	gs_Hwl = NULL;
}

IHwl* GetHWL()
{
	ASSERT(gs_Hwl!=NULL);
	return gs_Hwl;
}

static ImagerTester* gs_Tester=NULL;

void InitImagerTester()
{
	ASSERT(gs_Hwl!=NULL);
	const char *szEngine = gs_Hwl->EngineType();

	printf("-------- Engine type: %s ----------\n", szEngine);

	if (strcmp("JADE", szEngine) == 0)
		gs_Tester = new Gen6ImagerTester(gs_Hwl);
	else if (strcmp("MT9V022", szEngine) == 0)
		gs_Tester = new Gen5ImagerTester(gs_Hwl);
	else
		gs_Tester = new Gen5ImagerTester(gs_Hwl);	// act as a dummy for now

	gs_Tester->PowerUp();	// ensure the power state in the tester reflects the reality
	gs_Tester->LightsOff();
}

void TerminateImagerTester()
{
	delete gs_Tester;
	gs_Tester = NULL;
}

void ShowBitStatus(const char* szText, bool bit, bool expected)
{
	if(bit==expected)
	{
		printf("%s is %s", szText, bit ? "on" : "off");
	}
	else
	{
		printf("%s is broken: %s", szText, bit ? "on" : "off");
	}
}

void Execute_GetImageBuffers()
{
	//DBG_FUNC();
	const Image_buffer_pointers *pBuffers;
	size_t NumBuffers=0;
	pBuffers = gs_Hwl->GetImageBuffers(NumBuffers);
	for(size_t i=0; i<NumBuffers; i++)
	{
		printf("Buffer[%i]=%p\n", i, pBuffers[i].p_cached);
	}
}

void Execute_I2C_Tests()
{
	// Sensor test
	if(!gs_Tester->SensorTest())
		printf("!!! SensorTest FAILED !!!\n");
	else
		printf("*** SensorTest PASSED ***\n");

	// Psoc test
	if(!gs_Tester->PsocTest())
		printf("!!! PsocTest FAILED !!!\n");
	else
		printf("*** PsocTest PASSED ***\n");

	// Clock test
	if(!gs_Tester->ClockTest())
		printf("!!! ClockTest FAILED !!!\n");
	else
		printf("*** ClockTest PASSED ***\n");
}

void ShowHelp(char Rx)
{
	if(Rx!=0)
		printf("Unrecognized command: %c\n", Rx);
	puts("Commands:");
//	puts("  r   Read the next barcode");
//	puts("  R   Read continuously until a character from the debug port is received");
#ifdef HAS_FRAMEBUFFER
	puts("  s   Stream Images to LCD");
	puts("  i I Capture a single image and show it on LCD");
	puts("  l L Show the last image on LCD");
#endif

#ifndef NEW_AIMER_AND_ILLUMINATION_TEST
	puts("  a Aimer off");
	puts("  A Aimer on");
	puts("  i Illumination off");
	puts("  I Illumination on");
#else
	// TODO: We need to test the aimer/illumination on/off functionality
	//       as so.  This way we fully verify that get/set are working.
	puts("  a      Aimer on/off with GPIO");
	puts("  A      Aimer on/off with PSOC");
	puts("  i      Illumination on/off with GPIO");
	puts("  I      Illumination on/off with PSOC");
#endif

#if USE_SESSION
	puts("  c      connect session");
	puts("  C      disconnect session");
#endif

	puts("  s   Stream one Image to DEBUG");
	puts("  T   Stream 10 Images to DEBUG");
	puts("  S   Stream 2*10 Images to DEBUG");
	puts("  b   Execute GetImageBuffers");

	puts("  e   Execute i2c tests");
	puts("  E   Execute read/write sensor and PSOC read/write i2c");
	puts("  k   Execute read/write many sensor i2c (abort with keyhit)");
	puts("  K   Execute read/write many PSOC i2c (abort with keyhit)");
	puts("  p or P   Dump PSOC registers");

	puts("  u or U   Imager Power Up");
	puts("  d or D   Imager Power Down");

	puts("  p        Dump PSOC");
	puts("  P        Test Powerup/Down until keyhit");
	puts("  R        Reset the sensor");

	puts("  t        Test Normal Image");
	puts("  f or F   Test Pattern (fixed or horizontal shades)");
	puts("  m or M   Test Pattern (moving or vertical shades)");

	puts("  w Write debug images to files as PGM");
	puts("  W Write debug images to files as BMP");
//	puts("  v V Show the decoder version");
	puts("  q Q Exit");
	puts("  x X Exit");
}

#define NUM_IMAGES 10

void CyclePowerEnable()
{
	printf("cycle power enable\n");
	gs_Tester->PowerDown();
	gs_Tester->PowerUp();
}

void ShowTitle()
{
	const size_t Size=100;
	char Buffer[Size+1] = "???";
#if 0
	GetHWLayerRevision(Buffer, Size);
#endif
	printf("========== Kil Runner Rev %s, Kil Rev %s ===============\n", SVN_Revision, Buffer);
}

void KilRunner(void)
{
	ShowTitle();

	InitKil();
	InitImagerTester();
	InitImageDebug(GetHWL()->GetScanWidth(),GetHWL()->GetScanHeight());
#ifdef HAS_FRAMEBUFFER
	puts("Commands: a A i I s S w W q x (other characters show the help)\n$ ");
#else
	puts("Commands: a A i I s S d D u U w W q x (other characters show the help)\n$ ");
#endif
	ShowHelp(0);
	bool bRunning=true;
	while(bRunning)
	{
		printf("$ ");
		int Rx=getchar();
		puts(""); //CR
		switch(Rx)
		{
#if USE_SESSION
			case 'c':
				ASSERT(gs_Hwl!=NULL);
				if(!gs_Hwl->OpenSession())
				{
					puts("OpenSession failed");
				}
				break;
			case 'C':
				ASSERT(gs_Hwl!=NULL);
				gs_Hwl->CloseSession();
				break;
#endif
			case 'A':
				gs_Tester->TogglePsocAimer();
				break;
			case 'I':
				gs_Tester->TogglePsocIllumination();
				break;
			case 'a':
				gs_Tester->ToggleGpioAimer();
				break;
			case 'i':
				gs_Tester->ToggleGpioIllumination();
				break;
			case 'b':
				ASSERT(gs_Hwl!=NULL);
				Execute_GetImageBuffers();
				break;
			case 'd':
			case 'D':
				ASSERT(gs_Hwl!=NULL);
				//printf("ImagerPowerDown\n");
				gs_Tester->PowerDown();
				break;
			case 'p':
				gs_Tester->DumpPSOC();
				break;
			case 'e':
				Execute_I2C_Tests();
				break;
			case 'E':
				gs_Tester->ReadWriteI2c();
				break;
			case 'k':
				gs_Tester->ReadWriteI2c(true, false, 10000);
				break;
			case 'K':
				gs_Tester->ReadWriteI2c(false, true, 10000);
				break;

			// Test normal image
			case 't':
				printf("Test normal image with TEST_PATTERN_NONE\n");
				gs_Tester->SetCurrentTestPattern(TEST_PATTERN_NONE);
				break;

			// Test fixed pattern (fixed gray scale)
			case 'f':
			case 'F':
				printf("Test image with TEST_PATTERN_FIXED\n");
				gs_Tester->SetCurrentTestPattern(TEST_PATTERN_FIXED);
				break;

			// Test moving pattern (moving gray scale?)
			case 'm':
			case 'M':
				printf("Test image with TEST_PATTERN_MOVING\n");
				gs_Tester->SetCurrentTestPattern(TEST_PATTERN_MOVING);
				break;

///////////////////////////////////////////////////////////////////////////////
			case 'R':
				printf("ResetSensor\n");
				gs_Tester->ResetSensor();
				break;
			case 'P':
				ASSERT(gs_Hwl!=NULL);
				//printf("TestPowerDownUp\n");
				gs_Tester->TestPowerDownUp();
				break;
			case 'u':
			case 'U':
				ASSERT(gs_Hwl!=NULL);
				//printf("ImagerPowerUp\n");
				gs_Tester->PowerUp();
				break;
			case 'v':
			case 'V':
//				ShowVersions();
				break;
			case 'q':
			case 'Q':
			case 'x':
			case 'X':
//				oemDisconnect();
				bRunning=false;
				break;
#ifdef HAS_FRAMEBUFFER
			// Image
			case 'i':
			case 'I':
				DBG_RESET_IMAGE();
				ShowImage(true);
				break;
 			// Last image
			case 'l':
			case 'L':
				ShowImage(false);
				break;
			case 's':
			case 'S':
				DBG_RESET_IMAGE();
				StreamImages();
				Rx=getchar();
				break;
			case 'z':
				ShowStoredImage(true);                                // backwards
				break;
			case 'Z':
				ShowStoredImage(false);	                              // forward
				break;
#endif
			case 's':
				ResetImageDebug();
				gs_Tester->PowerDown();	// so we test whether "up" works
				gs_Tester->StreamImagesToFile(1);
				gs_Tester->PowerUp();
				break;

			case 'T':
				CyclePowerEnable();
				ResetImageDebug();
				gs_Tester->StreamImagesToFile(NUM_IMAGES);
				break;

			case 'S':
				ResetImageDebug();
				gs_Tester->PowerDown();	// so we test whether "up" works
				gs_Tester->StreamImagesToFile(NUM_IMAGES);
				gs_Tester->StreamImagesToFile(NUM_IMAGES);	// do it a second time to catch restart effects
				gs_Tester->PowerUp();
				break;

			case 'W':
				puts("Write BMP");
				WriteFileImageDebug(BMP);
				break;

			case 'w':
				puts("Write PGM");
				WriteFileImageDebug(PGM);
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
	TerminateKil();
	TerminateImagerTester();
	DBG_DESTROY_IMAGE();
}


int main( int argc, char ** argv )
{
	CTerminal NonCanonical;

//	OpenFramebuffer();
	KilRunner();
	puts("Terminated");
//	CloseFramebuffer();
	return 0;
}
