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

// Decoder.h : header file for Decoder API functions

#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
//  Various constants
//-----------------------------------------------------------------------------
#define DEC_MAX_RESONSE_SIZE				128
#define MAX_IMAGE_SIZE                  550000
#define MAX_MESSAGE_LENGTH              8192
#define WM_DECODE_EVENT_HWND_MSG        (WM_USER + 13031)


//-----------------------------------------------------------------------------
//  Defines for configuring desired OCR Templates using decSetOCRTemplates()
//-----------------------------------------------------------------------------
#define OCR_USER_TEMPLATE       0x0001  // User defined template. Enabled by default
#define OCR_PASSPORT_TEMPLATE   0x0002  // Internal passport template
#define OCR_ISBN_TEMPLATE       0x0004  // Internal ISBN template
#define OCR_PRICE_FIELD_TEMPLATE 0x0008 // Internal price field template
#define OCR_MICRE13B_TEMPLATE   0x0010  // Internal MicrE-13 B template


//-----------------------------------------------------------------------------
// These defines are used by decGetSetExposureSettings() and define the various
// exposure settings that can be modified and their ranges
//-----------------------------------------------------------------------------
#define DEC_ES_EXPOSURE_METHOD 0
#define DEC_ES_TARGET_VALUE 1
#define DEC_ES_TARGET_PERCENTILE 2
#define DEC_ES_TARGET_ACCEPT_GAP 3
#define DEC_ES_MAX_EXP 4
#define DEC_ES_MAX_GAIN 5
#define DEC_ES_FRAME_RATE 8
#define DEC_ES_CONFORM_IMAGE 9
#define DEC_ES_CONFORM_TRIES 10
#define DEC_ES_SPECULAR_EXCLUSION 11
#define DEC_ES_SPECULAR_SAT 12
#define DEC_ES_SPECULAR_LIMIT 13
#define DEC_ES_FIXED_EXP 14
#define DEC_ES_FIXED_GAIN 15
#define DEC_ES_FIXED_FRAME_RATE 16

#define DEC_MIN_ES_EXPOSURE_METHOD  0
#define DEC_MAX_ES_EXPOSURE_METHOD  2
#define DEC_MIN_ES_TARGET_VALUE 1
#define DEC_MAX_ES_TARGET_VALUE 255
#define DEC_MIN_ES_TARGET_PERCENTILE 1
#define DEC_MAX_ES_TARGET_PERCENTILE 99
#define DEC_MIN_ES_TARGET_ACCEPT_GAP 1
#define DEC_MAX_ES_TARGET_ACCEPT_GAP 50
#define DEC_MIN_ES_MAX_EXP 1
#define DEC_MAX_ES_MAX_EXP 7874
#define DEC_MIN_ES_MAX_GAIN 1
#define DEC_MAX_ES_MAX_GAIN 4
#define DEC_MIN_ES_FRAME_RATE 1
#define DEC_MAX_ES_FRAME_RATE 30
#define DEC_MIN_ES_CONFORM_IMAGE 0
#define DEC_MAX_ES_CONFORM_IMAGE 1
#define DEC_MIN_ES_CONFORM_TRIES 1
#define DEC_MAX_ES_CONFORM_TRIES 8
#define DEC_MIN_ES_SPECULAR_EXCLUSION 0
#define DEC_MAX_ES_SPECULAR_EXCLUSION 4
#define DEC_MIN_ES_SPECULAR_SAT 200
#define DEC_MAX_ES_SPECULAR_SAT 255
#define DEC_MIN_ES_SPECULAR_LIMIT 1
#define DEC_MAX_ES_SPECULAR_LIMIT 5
#define DEC_MIN_ES_FIXED_EXP 1
#define DEC_MAX_ES_FIXED_EXP 7874
#define DEC_MIN_ES_FIXED_GAIN 1
#define DEC_MAX_ES_FIXED_GAIN 4
#define DEC_MIN_ES_FIXED_FRAME_RATE 1
#define DEC_MAX_ES_FIXED_FRAME_RATE 30

#define DEC_AE_METHOD_UNIFORM 0
#define DEC_AE_METHOD_CENTER_ONLY 1
#define DEC_AE_METHOD_CENTER_WEIGHTED 2



//-----------------------------------------------------------------------------
//  AIM Symbology ID characters
//-----------------------------------------------------------------------------
#define AIMID_AZTEC         'z'
#define AIMID_CODABAR       'F'
#define AIMID_CODE11        'H'
#define AIMID_CODE128       'C'
#define AIMID_CODE39        'A'
#define AIMID_CODE49        'T'
#define AIMID_CODE93        'G'
#define AIMID_COMPOSITE     'e'
#define AIMID_DATAMATRIX    'd'
#define AIMID_EAN           'E'
#define AIMID_INT25         'I'
#define AIMID_MAXICODE      'U'
#define AIMID_MICROPDF      'L'
#define AIMID_PDF417        'L'
#define AIMID_OCR           'o'
#define AIMID_QR            'Q'
#define AIMID_RSS           'e'
#define AIMID_UPC           'E'
#define AIMID_POSTNET       'X'
#define AIMID_ISBT          'C'
#define AIMID_BPO           'X'
#define AIMID_CANPOST       'X'
#define AIMID_AUSPOST       'X'
#define AIMID_IATA25        'R'
#define AIMID_CODABLOCK     'O'
#define AIMID_JAPOST        'X'
#define AIMID_PLANET        'X'
#define AIMID_DUTCHPOST     'X'
#define AIMID_MSI           'M'
#define AIMID_TLC39         'L'
#define AIMID_TRIOPTIC      'X'
#define AIMID_CODE32		'X'
#define AIMID_STRT25		'S'
#define AIMID_MATRIX25		'X'
#define AIMID_PLESSEY		'P'
#define AIMID_CHINAPOST		'X'
#define AIMID_KOREAPOST		'X'
#define AIMID_TELEPEN		'B'
#define AIMID_CODE16K		'K'
#define AIMID_POSICODE		'p'
#define AIMID_COUPONCODE	'E'
#define AIMID_USPS4CB       'X'
#define AIMID_IDTAG			'X'
#define AIMID_LABEL			'X'
#define AIMID_GS1_128       'C'
#define AIMID_HANXIN        'X'
#define AIMID_GRIDMATRIX    'g'

//-----------------------------------------------------------------------------
//  Hand Held Products Symbology ID characters
//-----------------------------------------------------------------------------
#define SYMID_AZTEC         'z'
#define SYMID_CODABAR       'a'
#define SYMID_CODE11        'h'
#define SYMID_CODE128       'j'
#define SYMID_EAN128        'I'
#define SYMID_CODE39        'b'
#define SYMID_CODE49        'l'
#define SYMID_CODE93        'i'
#define SYMID_COMPOSITE     'y'
#define SYMID_DATAMATRIX    'w'
#define SYMID_EAN8          'D'
#define SYMID_EAN13         'd'
#define SYMID_INT25         'e'
#define SYMID_MAXICODE      'x'
#define SYMID_MICROPDF      'R'
#define SYMID_PDF417        'r'
#define SYMID_POSTNET       'P'
#define SYMID_OCR           'O'
#define SYMID_QR            's'
#define SYMID_RSS           'y'
#define SYMID_UPCA          'c'
#define SYMID_UPCE          'E'
#define SYMID_ISBT          'j'
#define SYMID_BPO           'B'
#define SYMID_CANPOST       'C'
#define SYMID_AUSPOST       'A'
#define SYMID_IATA25        'f'
#define SYMID_CODABLOCK     'q'
#define SYMID_JAPOST        'J'
#define SYMID_PLANET        'L'
#define SYMID_DUTCHPOST     'K'
#define SYMID_MSI           'g'
#define SYMID_TLC39         'T'
#define SYMID_TRIOPTIC      '='
#define SYMID_CODE32	    '<'
#define SYMID_STRT25	    'f'
#define SYMID_MATRIX25	    'm'
#define SYMID_PLESSEY		'n'
#define SYMID_CHINAPOST		'Q'
#define SYMID_KOREAPOST		'?'
#define SYMID_TELEPEN		't'
#define SYMID_CODE16K		'o'
#define SYMID_POSICODE		'W'
#define SYMID_COUPONCODE	'c'
#define SYMID_USPS4CB       'M'
#define SYMID_IDTAG			'N'
#define SYMID_LABELIV		'>'
#define SYMID_LABELV		','
#define SYMID_GS1_128       'I'
#define SYMID_HANXIN        'H'
#define SYMID_GRIDMATRIX    'x'


//-----------------------------------------------------------------------------
//  These are result return values for all functions
//-----------------------------------------------------------------------------
typedef enum
{
	RESULT_INITIALIZE = -1,
    RESULT_SUCCESS  = 0,				// Operation was successful
    RESULT_ERR_BADREGION,				// An image was requested using an invalid image region
    RESULT_ERR_DRIVER,					// Error detected in image engine driver
    RESULT_ERR_ENGINEBUSY,				// Image engine driver reported busy
    RESULT_ERR_MEMORY,					// Memory allocation failed
    RESULT_ERR_NODECODE,				// Image engine unable to decode a symbology
    RESULT_ERR_NOIMAGE,					// No image available
    RESULT_ERR_NORESPONSE,				// Could not communicate with imager
    RESULT_ERR_NOTCONNECTED,			// Not connected to image engine
    RESULT_ERR_PARAMETER,				// One of the function parameters was invalid
    RESULT_ERR_UNSUPPORTED,				// The operation was not supported by the engine
    RESULT_ERR_NOTRIGGER,				// Trigger state is false
    RESULT_ERR_BADSMARTIMAGE,			// IQ image fail
    RESULT_ERR_SMARTIMAGETOOLARGE,		// Requested IQ image too large
    RESULT_ERR_TOO_MUCH_INTERPOLATION,	// IQ image fail
	RESULT_ERR_WRONGRESULTSTRUCT,		// Invalid structure size
	RESULT_ERR_THREAD,					// Could not create async decode thread
	RESULT_ERR_CANCEL,					// Asynchronous decode was canceled
	RESULT_ERR_EXCEPTION,				// An exception was detected in the deoder
	RESULT_ERR_UNSUPPORTED_IQ_BARCODE,	// Scanned barcode is not a valid IQ host barcode
    RESULT_ERR_LOAD_EXMFILE,            // Error loading EXM file.
    RESULT_ERR_EXMFILE_INVALID,         // Not a valid configuration file.
    RESULT_ERR_MISSING_EXMSECTION,      // Section missing from exm file.
    RESULT_ERR_PROCESSING_EXMSECTION,   // Error processing exm file section.

}   Result_t;



//-----------------------------------------------------------------------------
// Symbology selection values
//-----------------------------------------------------------------------------
enum {
	SYM_AZTEC = 0,
    SYM_CODABAR,
    SYM_CODE11,
    SYM_CODE128,
    SYM_CODE39,
    SYM_CODE49,         // No longer supported starting in version 3.xx of Decoder DLL
    SYM_CODE93,
    SYM_COMPOSITE,
    SYM_DATAMATRIX,
    SYM_EAN8,
    SYM_EAN13,
    SYM_INT25,
    SYM_MAXICODE,
    SYM_MICROPDF,
    SYM_OCR,
    SYM_PDF417,
    SYM_POSTNET,
    SYM_QR,
    SYM_RSS,
    SYM_UPCA,
    SYM_UPCE0,
    SYM_UPCE1,
    SYM_ISBT,
    SYM_BPO,
    SYM_CANPOST,
    SYM_AUSPOST,
    SYM_IATA25,
    SYM_CODABLOCK,
    SYM_JAPOST,
    SYM_PLANET,
    SYM_DUTCHPOST,
    SYM_MSI,
    SYM_TLCODE39,
    SYM_TRIOPTIC,
	SYM_CODE32,
	SYM_STRT25,
	SYM_MATRIX25,
    SYM_PLESSEY,        // No longer supported starting in version 3.xx of Decoder DLL
	SYM_CHINAPOST,
	SYM_KOREAPOST,
	SYM_TELEPEN,
    SYM_CODE16K,        // No longer supported starting in version 3.xx of Decoder DLL
    SYM_POSICODE,       // No longer supported starting in version 3.xx of Decoder DLL
	SYM_COUPONCODE,
    SYM_USPS4CB,
	SYM_IDTAG,
    SYM_LABEL,         // No longer supported starting in version 3.xx of Decoder DLL
	SYM_GS1_128,
    SYM_HANXIN,
    SYM_GRIDMATRIX,     // No longer supported starting in version 3.xx of Decoder DLL
	SYM_POSTALS,		// Used to default and disable postal codes
    SYM_US_POSTALS1,    // Used to enable SYM_PLANET, SYM_POSTNET, SYM_USPS4CB & SYM_IDTAG
    NUM_SYMBOLOGIES,
    SYM_ALL=100
};




enum
{
    INTEL_IMG_MOVE_LEFT = 0,
    INTEL_IMG_MOVE_RIGHT,
    INTEL_IMG_MOVE_UP,
    INTEL_IMG_MOVE_DOWN,
    INTEL_IMG_BAD_CODEID,
    INTEL_IMG_ONE_SIDED_PDF,
    INTEL_IMG_OUT_OF_RANGE,
    INTEL_IMG_IS_NULL,
    INTEL_IMG_EVERYTHING_OK
};



//-----------------------------------------------------------------------------
//  Engine Identification
//-----------------------------------------------------------------------------
typedef enum
{
	UNKNOWN	= -1,
    NONE=0,
    IMAGER_4200_ENGINE=1,
    LASER_SE1200_ENGINE=2,
    LASER_SE1223_ENGINE=3,
	IMAGER_IT4000_ENGINE=5,
	IMAGER_IT4100_ENGINE=6,
	IMAGER_IT4300_ENGINE=7,
	IMAGER_IT5100_ENGINE=8,
	IMAGER_IT5300_ENGINE=9,
    IMAGER_N5603_ENGINE=12,
    IMAGER_N5600_ENGINE=13,
} EngineID_t;


//-----------------------------------------------------------------------------
//  Engine types
//-----------------------------------------------------------------------------
typedef enum
{
	ET_UNKNOWN = -1,
    ET_NONE=0,
    ET_IMAGER=1,
    ET_LASER=2
} EngineType_t;


//-----------------------------------------------------------------------------
//  Aimer Types
//-----------------------------------------------------------------------------
typedef enum
{
    AT_LASER = 0,
    AT_LED,
}   Aimer_t;

//-----------------------------------------------------------------------------
//  Supported OCR enable modes
//-----------------------------------------------------------------------------
typedef enum
{
    OCR_OFF = 0,
    OCR_NORMAL_VIDEO,
    OCR_INVERSE,
    OCR_BOTH
}   OCREnable_t;

//-----------------------------------------------------------------------------
// Query type
//-----------------------------------------------------------------------------
typedef enum
{
    QT_DEFAULT = 0,
    QT_CURRENT,
} QueryType_t;


//-----------------------------------------------------------------------------
// Lights mode used during decode function calls.
//-----------------------------------------------------------------------------
typedef enum
{
		LM_OFF                   = 0,
		LM_AIM_ON                = 1,
		LM_ILLUM_ON              = 2,
		LM_ILLUM_ON_AIM_ON       = (LM_ILLUM_ON | LM_AIM_ON),
		LM_CONCURRENT            = 4,
		LM_INTERLACED            = 5,
		LM_FLASH_deprecated      = 6,
		LM_BLINK                 = 7,
		LM_AIM_ONLY_NON_EXPOSURE = 8,
} LightsMode_t;

//-----------------------------------------------------------------------------
// Image formats for intelligent images
//-----------------------------------------------------------------------------
typedef enum
{
    IF_RAW_BINARY = 0,
    IF_RAW_GRAY,
} ImageFormat_t;


//----------------------------------------------------------------------------------
// Data structure that defines options that are used during decoding
//----------------------------------------------------------------------------------
typedef struct
{
	int           PrintWeight;        // Valid range is 1-7 - No longer supported starting in version 3.xx of Decoder DLL
	int			DecodeMode;			// Full Omni, Reduced Omni, ALD - No longer supported starting in version 3.xx of Decoder DLL
	int			LinearRange;		// Valid range 1-6 - No longer supported starting in version 3.xx of Decoder DLL
	int			VideoReverse;		// 0=disabled, 1=enabled - No longer supported starting in version 3.xx of Decoder DLL
	int			DecAttemptLimit;    // 0 = disabled, Valid range 1-10,000
	int			SearchLimit;		// 0 = disabled, Valid range 1-10,000
} DecodeOptions_t;

//----------------------------------------------------------------------------------
// Data structure that can be used to retrieve attributes of last acquired image
//----------------------------------------------------------------------------------
typedef struct
{
    int     Size;
    int		Exposure;
	int		Gain;
    int		IllumValue;
	int		IllumMax;
    int		IllumClip;
} ImageAttributes_t;


//------------------------------------------------------------------
//  Data structure for defining IQ imaging parameters
//------------------------------------------------------------------
typedef struct
{
    int AspectRatio;            // ratio of barcode height to narrow element width
    int OffsetX;                // offset in X direction, relative to barcode center
    int OffsetY;                // offset in Y direction
    unsigned int width;         // width of image in IntellBarcodeUnits
    unsigned int height;        // height of image
    int resolution;             // # pixels/IntellBarcodeUnits
    ImageFormat_t format;		// binary or grey scale
    int reserved;               // must be set to -1
} IntellImgDesc_t;


//------------------------------------------------------------------
//  Data structure that holds the decoded barcode message.
//------------------------------------------------------------------
typedef struct
{
    BYTE*  pchMessage;          // decoded message data
    BYTE   chCodeID;            // AIM Id of symbology
    BYTE   chSymLetter;         // HHP Id of symbology
    BYTE   chSymModifier;       // Modifier characters.
    WORD    nLength;            // length of the decoded message
} DecodeMsg_t;


//------------------------------------------------------------------
//  Data structure that defines the imager properties.
//------------------------------------------------------------------
typedef struct
{
	DWORD	dwSize;
	DWORD	dwEngineID;
    DWORD   dwImagerRows;
    DWORD   dwImagerCols;
    DWORD   dwBitsPerPixel;
	DWORD	dwRotation;
	DWORD	dwAimerXoffset;
	DWORD	dwAimerYoffset;
	DWORD	dwYDepth;
	DWORD   dwColorFormat;
	DWORD   dwNumBuffers;
	DWORD	dwFirmwareMajorRev;
	DWORD 	dwFirmwareMinorRev;
	BYTE	szSerialNo[81];
    DWORD   dwFirmwareEngineID; // Firmware engine ID.
    DWORD   dwAimerType;        // Aimer type. See HHP_AIMER_TYPE
    DWORD   dwAimerColor;       // Aimer color. See HHP_AIMER_COLOR
    DWORD   dwIllumColor;       // Illumination color. See HHP_ILLUM_COLOR
    DWORD   dwOptics;           // Optics type.  SEE HHP_SCANNER_OPTICS
    char    szPartNo[81];       // Engine part number

} ImagerProperties_t;



//------------------------------------------------------------------
//  Data structure that defines the decode centering window limits.
//------------------------------------------------------------------
typedef struct
{
    int	ULXMin;		// Upper left X allowable min value
	int	ULXMax;		// Upper left X allowable max value
    int	ULYMin;		// Upper left Y allowable min value
	int	ULYMax;		// Upper left Y allowable max value
	int	LRXMin;		// Lower right X allowable min value
	int	LRXMax;		// Lower right X allowable max value
    int	LRYMin;		// Lower right Y allowable min value
	int	LRYMax;		// Lower right Y allowable max value
} CenteringWindowLimits_t;


//----------------------------------------------------------------------------------------
// Exposure Mode types - NOTE: No longer supported starting in version 3.xx of Decoder DLL
//----------------------------------------------------------------------------------------

typedef enum {
	EXP_MODE_FIXED,
	EXP_MODE_ONCHIP,
	EXP_MODE_HHP
}ExposureMode_t;



#ifndef THREAD_CONFIG_H
#define THREAD_CONFIG_H
typedef enum {
	THREAD_CONFIG_BATCH,
	THREAD_CONFIG_STREAM,
	THREAD_CONFIG_CONCURRENT_IQ,
	THREAD_CONFIG_CONCURRENT_FAST,
	THREAD_CONFIG_CONCURRENT_FAST_IQ,
	THREAD_CONFIG_NUMBER
} ThreadConfig_t;
#endif




//======================================================================================
//  Decoder Engine API Functions
//======================================================================================

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


// General purpose functions
Result_t decAimerOn(BOOL bEnable);
Result_t decLightsOn(BOOL bEnable);

Result_t decConnect(void);
Result_t decDisconnect(void);

Result_t decGetErrorMessage(CHAR* pszErrorMsg, Result_t nError);
Result_t decGetMaxMessageChars(WORD *pnChars);
Result_t decGetEngineID( EngineID_t *pEngineID );
Result_t decGetEngineType( EngineType_t *pEngineType );
Result_t decGetAPIRevision(CHAR* pszRev);
Result_t decGetDecoderRevision(CHAR* pszRev);
Result_t decGetDecoderSecondaryRevision(CHAR* pszRev);
Result_t decGetDecoderControlRevision(CHAR* pszRev);
Result_t decGetDecThreadsRevision(CHAR* pszRev);
Result_t decGetScanDriverRevision(CHAR* pszRev);
//Result_t decGetHWLayerRevision(CHAR* pszRev);
Result_t decGetImagerProperties (ImagerProperties_t *pEngProps);

Result_t decGetPSOCMajorRev( int *pMajorRev );
Result_t decGetPSOCMinorRev( int *pMinorRev );
Result_t decGetEngineSerialNumber(CHAR* pszRev);

Result_t decConfigureFromExmFile(CHAR* pszExmFilename);

// Symbology related functions
Result_t decEnableDisableSymbology( int nSymId,BOOL bEnable );
Result_t decSetSymbologyDefaults( int nSymId );
Result_t decReadSymbologyMinMaxRange( int nSymId,DWORD* pMinVal, DWORD* pMaxVal );

Result_t decReadSymbologyConfig( QueryType_t Query, int nSymId, PVOID pSymStruct);
Result_t decWriteSymbologyConfig( int nSymId,PVOID pvSym );

Result_t decSetOCRUserTemplate( OCREnable_t nEnable, unsigned char* pszTemplate);
Result_t decSetOCRTemplates( DWORD nTemplate);
Result_t decSetOCRMode( OCREnable_t nEnable);

Result_t decWaitForDecode (DWORD dwTimeout, DecodeMsg_t *pRes, int (*fpCallBack)(void));

Result_t decWaitMultipleDecode(DWORD dwTimeout,
                               BOOL (*fpMultiReadCallBack)(DecodeMsg_t *),
                               BOOL (*fpKeepGoingCallBack)(void) );

Result_t decCancelAsyncDecode(void);
Result_t decGetAsyncDecodeResult(BYTE* pchMessage,
								 BYTE* pchCodeID,
								 BYTE* pchAIMID,
						         BYTE* pchSymModifier,
								 WORD* pnLength);


Result_t decGetSetDecodeCenteringWindow(BOOL bSet, QueryType_t QueryType, RECT *pIntersectRect );
Result_t decGetCenteringWindowLimits(CenteringWindowLimits_t *pLimits);
Result_t decEnableDecodeCenteringWindow(BOOL bEnable );

Result_t decGetSetDecodeOptions(BOOL Set,QueryType_t QueryType, DecodeOptions_t *pDecodeOptions);

Result_t decGetSetLightsMode(BOOL Set,QueryType_t QueryType, LightsMode_t *pLightsMode);

Result_t decGetLastImage( BYTE* pImageBuffer, DWORD* pdwSize, ImageAttributes_t* pImageAttributes );
Result_t decGetLastImageSize( WORD *pnCols, WORD *pnRows, DWORD *pdwSize );
Result_t decGetLastDecodeTime( DWORD * );

//Result_t decGetIntellImage(const IntellImgDesc_t* pImageDesc,byte *pImagePtr,DWORD *pImageSize);


// Functions for exposure modes and settings
Result_t decGetSetExposureMode(BOOL Set, ExposureMode_t *pExpMode);
Result_t decGetSetExposureSettings(BOOL Set, DWORD * pArray, DWORD dwArrayLength);

BOOL decScanInProgress(void);


Result_t decSetScanningConfig(ThreadConfig_t nMode);

// deprecated, only for existing apps.
Result_t decWaitForDecodeLegacy( DWORD dwTimeout, DecodeMsg_t *pRes, int (*fpCallBack)(void));

#ifdef __cplusplus
}
#endif  /* __cplusplus */


