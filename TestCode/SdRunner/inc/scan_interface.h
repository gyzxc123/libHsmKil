#ifndef scan_interface_h_cc03bfb1_72df_4788_9392_ac22b4a2b9e3
#define scan_interface_h_cc03bfb1_72df_4788_9392_ac22b4a2b9e3

#ifdef WINCE
#   include <windows.h>
#else
#   include "sdtypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef class Scan_session * S_HANDLE;
#ifndef WINCE
typedef S_HANDLE HANDLE;
#endif

typedef enum {
    HHPSD_ENGINE_IMAGER_OV7120,
    HHPSD_ENGINE_IMAGER_ICMEDIA,
    HHPSD_ENGINE_LINEAR_IMAGER_TOSHIBA,
    HHPSD_ENGINE_IMAGER_ALD_ICMEDIA,
    HHPSD_ENGINE_IMAGER_MI1300,
    HHPSD_ENGINE_IMAGER_STM_VC700,
    HHPSD_ENGINE_IMAGER_STM_VC602,
    HHPSD_ENGINE_IMAGER_MICRON_MT9V022,
    HHPSD_ENGINE_IMAGER_E2V_JADE,
    HHPSD_ENGINE_IMAGER_MICRON_MT9P031 //FIXME_IMX53
} HHPScanDriverEngineType_t;

typedef struct _tagImageAttributes
{
    DWORD dwSize;
    DWORD nExposure;
    DWORD nGain;
    DWORD nIllumValue;
    DWORD nIllumMaxValue;
    DWORD nHeight;
    DWORD nWidth;
    DWORD nImageNumber;
    DWORD nMonocolorBlueX;
    DWORD nMonocolorBlueY;
} ImageAttributes;

typedef enum {
    HHPSD_IMAGE_STAT_EXPOSURE,
    HHPSD_IMAGE_STAT_GAIN,
    HHPSD_IMAGE_STAT_ILLUM_VALUE,
    HHPSD_IMAGE_STAT_ILLUM_MAXIMUM,
    HHPSD_IMAGE_STAT_ILLUM_CLIP,
    HHPSD_IMAGE_STAT_HEIGHT,
    HHPSD_IMAGE_STAT_WIDTH,
    HHPSD_IMAGE_STAT_IMAGE_NUMBER,
    HHPSD_IMAGE_STAT_IQ_SCORE,
    HHPSD_IMAGE_BLUE_X,
    HHPSD_IMAGE_BLUE_Y,
} HHPScanDriverImageAttributes_t;

typedef struct
{
    DWORD    dwSize;
    DWORD    dwEngineID;
    DWORD   dwImagerRows;
    DWORD   dwImagerCols;
    DWORD   dwBitsPerPixel;
    DWORD    dwRotation;
    DWORD    dwAimerXoffset;
    DWORD    dwAimerYoffset;
    DWORD    dwYDepth;

} HHP_SCANENGINE_PROPERTIES, *PHHP_SCANENGINE_PROPERTIES;

typedef struct
{
    // First 9 items in this structure must match first 9 in
    // HHP_SCANENGINE_PROPERTIES.
    DWORD    dwSize;
    DWORD    dwEngineID;
    DWORD   dwImagerRows;      ///< Number of rows in an image
    DWORD   dwImagerCols;      ///< Number of columns in an image
    DWORD   dwBitsPerPixel;
    DWORD    dwRotation;
    DWORD    dwAimerXoffset;
    DWORD    dwAimerYoffset;
    DWORD    dwYDepth;
    DWORD   dwColorFormat;
    DWORD   dwNumBuffers;     ///< The number of image buffers allocated
                              //   by scan driver
    DWORD   dwFirmwareMajorRev; ///< Firmware major revision
    DWORD   dwFirmwareMinorRev; ///< Firmware minor revision
    char    szSerialNo[81];     ///< Engine serial number, null terminated.


}  HHP_SCANENGINE_PROPERTIES_EX, *PHHP_SCANENGINE_PROPERTIES_EX;

typedef struct _tagHHP_SCANNER_LIGHTING
{
    enum LightsMode
    {
        OFF                   = 0,
        AIM_ON                = 1,
        ILLUM_ON              = 2,
        ILLUM_ON_AIM_ON       = (ILLUM_ON | AIM_ON),
        CONCURRENT            = 4,
        INTERLACED            = 5,
        FLASH_deprecated      = 6,
        BLINK                 = 7,
        AIM_ONLY_NON_EXPOSURE = 8,
    };

    enum IlluminationSource
    {
        PRIMARY       = 0,
        ALTERNATE     = 1
    };

    DWORD  dwSize;
    DWORD  dwScanMode;
    DWORD  dwIdleMode;
    DWORD  dwIlluminationSource;
    DWORD  dwAimerIntensity;
    DWORD  dwIlluminationIntensity;
    BOOL   bNoBoost; ///< If TRUE, boost lights are not used.
} HHP_SCANNER_LIGHTING, *PHHP_SCANNER_LIGHTING;

typedef enum
{
    ES_EXPOSURE_METHOD,        /* How should we do this */
    ES_TARGET_VALUE,        /* This is the target pixel count we shoot for */
    ES_TARGET_PERCENTILE,    /* This goes along with Value, the target value should be at this percentile */
    ES_TARGET_ACCEPT_GAP,    /* How close to the target value must we be */
    ES_MAX_EXP,                /* This is the maximum exposure we're allowed to use */
    ES_MAX_GAIN,            /* This is the maximum gain we're allowed to use */
    ES_STARTING_EXP,        /* When scanning starts, this is the first exposure that we try (updated when scanning stops)*/
    ES_STARTING_GAIN,        /* When scanning starts, this is the first gain that we try (updated when scanning stops)*/
    ES_FRAME_RATE,            /* This is essentially the frame rate to use, I call it divide because that's what it does */
    ES_CONFORM_IMAGE,        /* The image must conform to the auto-exposure requirements, if not, it's rejected. */
    ES_CONFORM_TRIES,        /* The number of times we'll attempt to conform. */
    ES_SPECULAR_EXCLUSION,    /* Do we exclude specular? to what degree? */
    ES_SPECULAR_SAT,        /* These next two only come into play if somebody specifies nSpecularExclusion*/
    ES_SPECULAR_LIMIT,        /* to be HHPSD_SPECULAR_EXCLUSION_SPECIAL. */
    ES_FIXED_EXP,
    ES_FIXED_GAIN,
    ES_FIXED_FRAME_RATE,
    ES_ADJUST_EXP_WHEN_ILLUM_OFF,
    ES_DECODE_PARAMETER,
    ES_FAST_START,
    ES_MAX_EXP_US,
    ES_MAX_GAIN_256,
    ES_FIXED_EXP_US,        /* Fixed exposure in microsecond units. */
    ES_FIXED_GAIN_256,      /* Gain in 1/256 units. (512 is a gain of 2x) */
    ES_NUM_EXPOSURE_ITEMS,
} HHP_IMAGER_EXPOSURE_TAG;

enum HHPScanDriverLightMode_t
{
    HHPSD_LM_OFF             = 0,
    HHPSD_LM_AIM_ON          = 1,
    HHPSD_LM_ILLUM_ON        = 2,
    HHPSD_LM_ILLUM_ON_AIM_ON = (HHPSD_LM_ILLUM_ON | HHPSD_LM_AIM_ON),
    HHPSD_LM_CONCURRENT      = 4,
    HHPSD_LM_INTERLACED      = 5,
    HHPSD_LM_FLASH_deprecated = 6,
    HHPSD_LM_BLINK           = 7,
    HHPSD_LM_AIM_ONLY_NON_EXPOSURE = 8,
    HHPSD_LM_NUM_MODES // Must be last.
};

typedef enum {
    HHPSD_AE_METHOD_UNIFORM,
    HHPSD_AE_METHOD_CENTER_ONLY,
    HHPSD_AE_METHOD_CENTER_WEIGHTED
} HHPScanDriverHHPExposureMethod;

typedef enum {
    HHPSD_SPECULAR_EXCLUSION_OFF,
    HHPSD_SPECULAR_EXCLUSION_MINIMAL,
    HHPSD_SPECULAR_EXCLUSION_MODERATE,
    HHPSD_SPECULAR_EXCLUSION_AGGRESSIVE,
    HHPSD_SPECULAR_EXCLUSION_SPECIAL
} HHPScanDriverHHPExposureSpecularExclusion;

typedef enum {
    HHPSD_EXPOSURE_FIXED,
    HHPSD_AUTOEXPOSURE_USE_ONCHIP,
    HHPSD_AUTOEXPOSURE_USE_HHP,
    HHPSD_AUTOEXPOSURE_HHP_PRESENTATION,
    HHPSD_CONTEXT_SENSITIVE_USE_HHP,
    HHPSD_OPENLOOP_GAIN,
    HHPSD_CELLPHONE,
    HHPSD_AUTOEXPOSURE_DUAL_TRACK_USE_HHP,
    HHPSD_CONTEXT_SENSITIVE_DUAL_TRACK_USE_HHP,
}HHPScanDriverAutoExposureMode_t;

struct Exposure_settings
{
    DWORD dwSize;
    DWORD /*HHPScanDriverAutoExposureMode_t*/ nAutoExposureMode;
    DWORD /*HHPScanDriverHHPExposureMethod*/ MethodToUse; /* How should we do this */
    DWORD nTargetValue;            /* This is the target pixel count we shoot for */
    DWORD nTargetPercentile;    /* This goes along with Value, the target value should be at this percentile */
    DWORD nTargetAcceptanceGap;    /* How close to the target value must we be */
    DWORD nMaximumExposure;        /* This is the maximum exposure we're allowed to use */
    DWORD nMaximumGain;            /* This is the maximum gain we're allowed to use */
    DWORD nStartingExposure;    /* When scanning starts, this is the first exposure that we try (updated when scanning stops)*/
    DWORD nStartingGain;        /* When scanning starts, this is the first gain that we try (updated when scanning stops)*/
    DWORD nFrameRate;
    DWORD  bImageMustConform;    /* The image must conform to the auto-exposure requirements, if not, it's rejected. */
    DWORD nTriesForConforming;    /* If bImageMustConform is set, this is the number of images that will be taken before we give up
                                 * trying to conform.  We don't want to lock up the system trying to get a conforming image*/
    DWORD /*HHPScanDriverHHPExposureSpecularExclusion*/  nSpecularExclusion;    /* Do we exclude specular? to what degree? */
    DWORD nSpecialSpecularSaturation; /* These next two only come into play if somebody specifies nSpecularExclusion*/
    DWORD nSpecialSpecularLimit;      /* to be HHPSD_SPECULAR_EXCLUSION_SPECIAL. */
    DWORD bAdjustExpWhenIllumOff; /* If the illumination is being turned off and on during a scanning session, do we want the exposure adjusted
                                             * when the illumination is off */

    DWORD nFixedExposure;
    DWORD nFixedGain;
    DWORD nFixedFrameRate;
};


bool HHPSI_InitializeScanEngine(void);


HANDLE HHPSI_OpenSessionWithScanEngine(void);
void HHPSI_CloseSessionWithScanEngine(HANDLE hSession);

void * HHPSI_GetNewScan(HANDLE hSession);
bool HHPSI_StartScanning(HANDLE hSession);
bool HHPSI_StopScanning(HANDLE hSession);

bool HHPSI_UnlockBuffer(HANDLE hSession, void * buffer);
bool HHPSI_LockBuffer(HANDLE hSession, void * buffer);
bool  HHPSI_SetExposureMode( HANDLE hSession,DWORD dwMode);
bool HHPSI_SetExposureSettings( HANDLE hSession,const DWORD *pdwArray, 
                                DWORD dwArrayLength              );
bool HHPSI_GetExposureSettings( HANDLE hSession,DWORD *pdwArray, 
                                DWORD dwArrayLength);


void * HHPSI_GetSingleFrame(HANDLE hSession);

bool HHPSI_SetLights(HANDLE hSession, const HHP_SCANNER_LIGHTING * pLights);
bool HHPSI_GetLights(HANDLE hSession, HHP_SCANNER_LIGHTING * pLights);

bool HHPSI_GetScanEngineProperties( HANDLE hSession,
                                    PHHP_SCANENGINE_PROPERTIES pScnEngProp );
bool HHPSI_GetScanEnginePropertiesEx(HANDLE hSession, 
                                     PHHP_SCANENGINE_PROPERTIES_EX p_pEngProps,
                                     DWORD dwSzProperties                   );

bool GetImagerProperties(PHHP_SCANENGINE_PROPERTIES pScnEngProp );
bool GetImagerPropertiesEx(  PHHP_SCANENGINE_PROPERTIES_EX pEngProps,
                              DWORD dwSzProperties                     );

bool HHPSI_GetDllRevision(TCHAR* pszRev);
const char * HHPSI_RevisionString();

bool  HHPSI_GetScanDriverRevision(HANDLE hSession, TCHAR* pszRev);
bool  HHPSI_GetImageAttributes( HANDLE hSession, void *pImage, 
                                DWORD *pdwArray, DWORD dwArrayLength );
bool  HHPSI_GetImageAttributesEx(  HANDLE hSession, void *pImage, 
                                   ImageAttributes * pAttributes,
                                   DWORD dwAttributesLength           );
bool  GetEngineType(BYTE *pEngineType);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef scan_interface_h_cc03bfb1_72df_4788_9392_ac22b4a2b9e3
