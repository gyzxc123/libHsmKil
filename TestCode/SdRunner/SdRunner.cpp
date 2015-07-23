/*===================================================================
  Debug code. Use this to mess around and keep SimpleBarcodeReader clean.

   $Source: Linux/Android/TestCode/DebugRunner/DebugRunner.cpp $
   $Author: Fauth,Dieter (E411776) $
   $Date: 2012/06/22 11:18:22EDT $
   $Revision: 1.8 $

   Copyright I Honeywell, Inc. 2008
===================================================================*/


#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <Os.h>
#include <DebugHelpers.h>
#include <ImageDebugInterface.h>
#include <scan_interface.h>
#include <logging.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////
// debug helpers
// Set to 1 if you want to store the last image to a file
//#define AUTO_STORE_LAST_IMAGE    1

static bool bStore=true;

//#include <android/log.h>
//#define LOGMSG(x)  __android_log_print x
// example: LOGMSG((ANDROID_LOG_INFO, "MyModuleName", "My Message"));
// example: LOGMSG((ANDROID_LOG_INFO, "MyModuleName", "My Message%i", variable));
// view logs: adb logcat -d
// clear logs: adb logcat -c

static HANDLE hSession=NULL;
static DWORD ExposureValues[] =
{
    ES_FIXED_GAIN_256,
    256,
    ES_FIXED_EXP_US,
    100
};

void InitSD()
{
    hSession = HHPSI_OpenSessionWithScanEngine();
    if(hSession)
    {
        bool bExp = HHPSI_SetExposureSettings(hSession, ExposureValues, sizeof(ExposureValues));
        bool bMod = HHPSI_SetExposureMode(hSession, HHPSD_AUTOEXPOSURE_USE_HHP);
        if(!bExp)
            printf("HHPSI_SetExposureSettings failed\n");
        if(!bMod)
            printf("HHPSI_SetExposureMode failed\n");
    }
    else
    {
        printf("HHPSI_OpenSessionWithScanEngine failed\n");
    }
}

void TerminateSD()
{
    if(hSession)
    {
        HHPSI_CloseSessionWithScanEngine(hSession);
    }
}

void InitImageDebug()
{
    HHP_SCANENGINE_PROPERTIES_EX EngProps;
    EngProps.dwSize = sizeof(HHP_SCANENGINE_PROPERTIES);
    bool Success = HHPSI_GetScanEnginePropertiesEx (hSession, &EngProps,EngProps.dwSize);
    if(Success)
    {
        InitImageDebug(EngProps.dwImagerCols, EngProps.dwImagerRows);
    }
    else
    {
        printf("InitImageDebug failed\n");
    }
}

void StreamImagesToFile(int NumImages)
{
    bool status = HHPSI_StartScanning(hSession);
    if(!status)
        printf("HHPSI_StartScanning failed\n");
    printf("HHPSI_StartScanning %i\n", (int) status);

    if(status)
    {
        for(int i=0; i<NumImages; i++)
        {
            printf("Calling HHPSI_GetNewScan\n");
            void* ptr = HHPSI_GetNewScan(hSession);
            if(ptr)
            {
                if(bStore)
                {
                    StoreImageDebug((unsigned char *) ptr, DBG_STAMP_SD);
                }
                status = HHPSI_UnlockBuffer(hSession, ptr);
                if(!status)
                    printf("HHPSI_UnlockBuffer failed\n");
            }
            else
            {
                printf("HHPSI_GetNewScan failed\n");
            }
        }
    }

    printf("Calling HHPSI_StopScanning\n");
    status = HHPSI_StopScanning(hSession);
    if(!status)
        printf("HHPSI_StopScanning failed\n");
}

void StreamImagesToFile_UI(int NumImages)
{
    printf("Stream %i images to array\n", NumImages);
    StreamImagesToFile(NumImages);
}

void LongRun(bool bStoreAll)
{
    const int ImagesPerRound=5;
    int Rounds=0;
    SetImageDebugQuiet(true);
    while(!AbortOnKbHit())
    {
        bStore=bStoreAll;
        if (Rounds % 200 == 0)
        {
            printf("Rounds so far = %u\n", Rounds);
            bStore=true;
        }
        Rounds++;
        StreamImagesToFile(ImagesPerRound);
        if(bStore)
        {
            WriteFileImageDebug(PGM);
            ResetImageDebug();
        }
        //usleep(200*1000);
    };
    printf("Exit with %u rounds\n", Rounds);
}

void ShowHelp(char Rx)
{
    if(Rx!=0)
        printf("Unrecognized command: %c\n", Rx);
    puts("Commands:");
#ifdef HAS_FRAMEBUFFER
    puts("  s   Stream Images to LCD");
    puts("  i I Capture a single image and show it on LCD");
    puts("  l L Show the last image on LCD");
#endif
    puts("  s   Stream 2*1 Image to DEBUG");
    puts("  S   Stream 2*10 Images to DEBUG");
    puts("  r   Stream 5 Images repeatedly (store some to DEBUG)");
    puts("  R   Stream 5 Images repeatedly (store all to DEBUG)");

    puts("  w Write debug images to files as BMP");
    puts("  W Write debug images to files as PGM bytes");
//    puts("  v V Show the decoder version");
    puts("  q Q Exit");
    puts("  x X Exit");
}

void ShowTitle()
{
    printf("========== SD Runner Revision %s, SD %s ============\n", SVN_Revision, HHPSI_RevisionString());
}

void SdRunner(void)
{
    ShowTitle();
    InitSD();
    InitImageDebug();
#ifdef HAS_FRAMEBUFFER
    puts("Commands: s S w q x (other characters show the help)\n$ ");
#else
    puts("Commands: s S w q x (other characters show the help)\n$ ");
#endif
    ShowHelp(0);
    while(1)
    {
        printf("$ ");
        int Rx=getchar();
        switch(Rx)
        {
            case 'v':
            case 'V':
//                ShowVersions();
                break;
            case 'q':
            case 'Q':
            case 'x':
            case 'X':
                TerminateSD();
                return;
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
                ShowStoredImage(false);                                  // forward
                break;
#endif
            case 's':
                DBG_RESET_IMAGE();
                StreamImagesToFile_UI(1);
                StreamImagesToFile_UI(1);
//                Rx=getchar();
                break;
            case 'S':
                DBG_RESET_IMAGE();
                StreamImagesToFile_UI(10);
                StreamImagesToFile_UI(10);
//                Rx=getchar();
                break;
            case 'r':
                DBG_RESET_IMAGE();
                LongRun(false);
//                Rx=getchar();
                break;
            case 'R':
                DBG_RESET_IMAGE();
                LongRun(true);
//                Rx=getchar();
                break;
            case 'w':
                DBG_WRITE_IMAGE_FILES(BMP);
                break;
            case 'W':
                DBG_WRITE_IMAGE_FILES(PGM);
                break;
//            case 'c':
//            case 'C':
//                oemDisconnect();
//                CaptureImageAndShow();
//                RetVal = oemConnect();
//                break;
            case '\r':
            case '\n':
                puts("");
                break;
            default:
                ShowHelp(Rx);
                break;
        }
        usleep(20*1000);
    }
}

int main( int argc, char ** argv )
{
    CTerminal NonCanonical;

//    OpenFramebuffer();
    SdRunner();
    DBG_DESTROY_IMAGE();
    puts("Terminated");
//    CloseFramebuffer();

    return 0;
}
