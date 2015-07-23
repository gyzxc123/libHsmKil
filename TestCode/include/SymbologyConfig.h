//======================================================================================
// SymbologyConfig.h  - contains symbology config defines, etc
// Copyright (c) 2006 HHP Inc. All Rights Reserved.
//======================================================================================
// $RCSfile: DecoderDll/SymbologyConfig.h $
// $Revision: 1.10 $
// $Date: 2009/08/17 15:46:02EDT $
//======================================================================================

#ifndef SYMBOLOGY_CONFIG_H

    #define SYMBOLOGY_CONFIG_H

	//-----------------------------------------------------------------------------
    // Decoder configuration definitions for each symbology
    #define SYMBOLOGY_ENABLE                0x00000001  // Enable Symbology bit
    #define SYMBOLOGY_CHECK_ENABLE          0x00000002  // Enable usage of check character
    #define SYMBOLOGY_CHECK_TRANSMIT        0x00000004  // Send check character
    #define SYMBOLOGY_START_STOP_XMIT       0x00000008  // Include the start and stop characters in the decoded result string
    #define SYMBOLOGY_ENABLE_APPEND_MODE    0x00000010  // Code39 append mode
    #define SYMBOLOGY_ENABLE_FULLASCII      0x00000020  // Enable Code39 Full ASCII
    #define SYMBOLOGY_NUM_SYS_TRANSMIT      0x00000040  // UPC-A/UPC-e Send Num Sys
    #define SYMBOLOGY_2_DIGIT_ADDENDA       0x00000080  // Enable 2 digit Addenda (UPC & EAN)
    #define SYMBOLOGY_5_DIGIT_ADDENDA       0x00000100  // Enable 5 digit Addenda (UPC & EAN)
    #define SYMBOLOGY_ADDENDA_REQUIRED      0x00000200  // Only allow codes with addenda (UPC & EAN)
    #define SYMBOLOGY_ADDENDA_SEPARATOR     0x00000400  // Include Addenda separator space in returned string.
    #define SYMBOLOGY_EXPANDED_UPCE         0x00000800  // Extended UPC-E
    #define SYMBOLOGY_UPCE1_ENABLE          0x00001000  // UPC-E1 enable (use SYMBOLOGY_ENABLE for UPC-E0)
    #define SYMBOLOGY_COMPOSITE_UPC         0x00002000  // Enable UPC composite codes
    #define SYMBOLOGY_AZTEC_RUNE            0x00004000  // Enable Aztec Run
    #define SYMBOLOGY_AUSTRALIAN_BAR_WIDTH  0x00010000  // Include australian postal bar data in string
    #define SYMBOLOGY_UPC_FAR				0x00020000  // Enable UPC "far" decoding
    #define SYMBOLOGY_128_FAR				0x00040000  // Enable Code 128 "far" decoding
    #define SYMBOLOGY_128_APPEND			0x00080000  // Enable OR dISBALE Code 128 FNC2 append functionality

    // For RSE,RSL,RSS there is only one symbology ID so we use 3 flags for enable
    #define SYMBOLOGY_RSE_ENABLE            0x00800000  // Enable RSE Symbology bit
    #define SYMBOLOGY_RSL_ENABLE            0x01000000  // Enable RSL Symbology bit
    #define SYMBOLOGY_RSS_ENABLE            0x02000000  // Enable RSS Symbology bit
    #define SYMBOLOGY_RSX_ENABLE_MASK       0x03800000
    // Telepen and PosiCode
    #define SYMBOLOGY_TELEPEN_OLD_STYLE     0x04000000  // Telepen Old Style mode.
    #define SYMBOLOGY_POSICODE_LIMITED_1    0x08000000  // PosiCode Limited of 1
    #define SYMBOLOGY_POSICODE_LIMITED_2    0x10000000  // PosiCode Limited of 2
    #define SYMBOLOGY_CODABAR_CONCATENATE   0x20000000  // Codabar concatenate.

    // Symbology structure set masks
    #define SYM_MASK_FLAGS                  0x00000001  // Flags are valid
    #define SYM_MASK_MIN_LEN                0x00000002  // Min Length valid
    #define SYM_MASK_MAX_LEN                0x00000004  // Max Length valid
    #define SYM_MASK_ALL                    0x00000007	// All fields valid

    // Structure for symbologies with no specified min or max length.
    typedef struct _tagSymFlagsOnly
    {
        DWORD   dwStructSize;                           // Set to sizeof( SymFlagsOnly_t );
        DWORD   dwMask;
        DWORD   dwFlags;

    } SymFlagsOnly_t, *PSymFlagsOnly_t;

    // Structure for symbologies with min & max length.
    typedef struct _tagSymFlagsRange
    {
        DWORD   dwStructSize;                           // Set to sizeof( SymFlagsRange_t );
        DWORD   dwMask;
        DWORD   dwFlags;
        DWORD   dwMinLen;
        DWORD   dwMaxLen;

    } SymFlagsRange_t, *PSymFlagsRange_t;


    // Define aliases for each symbology structure
    #define AZTEC_T         SymFlagsRange_t
    #define CODABAR_T       SymFlagsRange_t
    #define CODE11_T        SymFlagsRange_t
    #define CODE128_T       SymFlagsRange_t
    #define CODE39_T        SymFlagsRange_t
    #define CODE49_T        SymFlagsRange_t
    #define CODE93_T        SymFlagsRange_t
    #define COMPOSITE_T     SymFlagsRange_t
    #define DATAMATRIX_T    SymFlagsRange_t
    #define EAN8_T          SymFlagsOnly_t
    #define EAN13_T         SymFlagsOnly_t
    #define INT25_T         SymFlagsRange_t
    #define MAXICODE_T      SymFlagsRange_t
    #define MICROPDF_T      SymFlagsRange_t
    #define PDF417_T        SymFlagsRange_t
    #define POSTNET_T       SymFlagsOnly_t
    #define QR_T            SymFlagsRange_t
    #define RSS_T           SymFlagsRange_t
    #define UPCA_T          SymFlagsOnly_t
    #define UPCE_T          SymFlagsOnly_t
    #define ISBT_T          SymFlagsOnly_t
    #define BPO_T           SymFlagsOnly_t
    #define CANPOST_T       SymFlagsOnly_t
    #define AUSPOST_T       SymFlagsOnly_t
    #define IATA25_T        SymFlagsRange_t
    #define CODABLOCK_T     SymFlagsRange_t
    #define JAPOST_T        SymFlagsOnly_t
    #define PLANET_T        SymFlagsOnly_t
    #define DUTCHPOST_T     SymFlagsOnly_t
    #define MSI_T           SymFlagsRange_t
    #define TLCODE39_T      SymFlagsOnly_t
    #define MATRIX25_T      SymFlagsRange_t
    #define KORPOST_T       SymFlagsRange_t
    #define TRIOPTIC_T      SymFlagsOnly_t
    #define CODE32_T        SymFlagsOnly_t
    #define CODE25_T        SymFlagsRange_t
    #define PLESSEY_T       SymFlagsRange_t
    #define CHINAPOST_T     SymFlagsRange_t
    #define TELEPEN_T       SymFlagsRange_t
    #define CODE16K_T       SymFlagsRange_t
    #define POSICODE_T      SymFlagsRange_t
    #define COUPONCODE_T    SymFlagsOnly_t
	#define USPS4STATE_T	SymFlagsOnly_t
	#define IDTAG_T			SymFlagsOnly_t
	#define LABELCODE_T		SymFlagsOnly_t
    #define GS1_128_T       SymFlagsRange_t
    #define HX_T			SymFlagsRange_t
    #define GM_T			SymFlagsRange_t
    // Structure of structures, one for each symbology.
    typedef struct _tagSymCfg
    {
        DWORD           dwStructSize;   // Set to sizeof( SymCfg_t );
        // Linear Codes                 // Flags supported for this code
        //---------------------------------------------------------------
        CODABAR_T       codabar;        // Enable,Check,CheckSend,StartStop,Concatenate
        CODE11_T        code11;         // Enable,Check,CheckSend
        CODE128_T       code128;        // Enable
        CODE39_T        code39;         // Enable,Check,CheckSend,StartStop,Append,FullAscii
        CODE49_T        code49;         // Enable
        CODE93_T        code93;         // Enable
        COMPOSITE_T     composite;      // Enable,CompositeUPC
        DATAMATRIX_T    datamatrix;     // Enable
        EAN8_T          ean8;           // Enable,Check,Addenda2,Addenda5,AddendaReq,AddendaSep
        EAN13_T         ean13;          // Enable,Check,Addenda2,Addenda5,AddendaReq,AddendaSep
        IATA25_T        iata25;         // Enable
        INT25_T         int2of5;        // Enable,Check,CheckSend
        ISBT_T          isbt;           // Enable
        MSI_T           msi;            // Enable,Check
        UPCA_T          upcA;           // Enable,check,NumSysTrans,Addenda2,Addenda5,AddendaReq,AddendaSep
        UPCE_T          upcE;           // Enable,check,NumSysTrans,Addenda2,Addenda5,AddendaReq,AddendaSep,ExpandedE,EnableE1
        // Postal Codes
        AUSPOST_T       australiaPost;  // Enable,AustralianBar
        BPO_T           britishPost;    // Enable
        CANPOST_T       canadaPost;     // Enable
        DUTCHPOST_T     dutchPost;      // Enable
        JAPOST_T        japanPost;      // Enable
        PLANET_T        usPlanet;       // Enable,Check
        POSTNET_T       usPostnet;      // Enable,Check
        // 2D Codes
        AZTEC_T         aztec;          // Enable,AztecRune
        CODABLOCK_T     codablock;      // Enable
        MAXICODE_T      maxicode;       // Enable
        MICROPDF_T      microPDF;       // Enable
        PDF417_T        pdf417;         // Enable
        QR_T            qr;             // Enable
        RSS_T           rss;            // Enable (RSS,RSL,RSE)
        TLCODE39_T      tlCode39;       // Enable
        // New codes
        MATRIX25_T      matrix25;       // Enable,
        KORPOST_T       koreaPost;      // Enable
        TRIOPTIC_T      triopticCode;   // Enable
        CODE32_T        code32;         // Enable
        CODE25_T        code2of5;       // Enable
        PLESSEY_T       plesseyCode;    // Enable
        CHINAPOST_T     chinaPost;      // Enable
        TELEPEN_T       telepen;        // Enable,OldStyle?
        CODE16K_T       code16k;        // Enable
        POSICODE_T      posiCode;       // Enable,Limited 1 and 2
        COUPONCODE_T    couponCode;     // Enable
        USPS4STATE_T	usps4Post;		// Enable
        IDTAG_T		    idtagPost;      // Enable
		LABELCODE_T		labelCode;		// Enable
        GS1_128_T       gs1_128;        // Enable
        HX_T			hx;			    // Enable
		GM_T			gm;				// Enable
    } SymCfg_t, *PSymCfg_t;

#endif

