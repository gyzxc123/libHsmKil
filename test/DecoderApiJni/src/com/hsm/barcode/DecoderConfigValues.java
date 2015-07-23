package com.hsm.barcode;
/**
 * These are the various classes used for configuring the decoder functionality
 */
public final class DecoderConfigValues {
	private DecoderConfigValues(){
		
	}
	
	public final static class SymbologyID{
		private SymbologyID(){
		}
		public static final int SYM_AZTEC = 0;
	    public static final int SYM_CODABAR = 1;
	    public static final int SYM_CODE11 = 2;
	    public static final int SYM_CODE128 = 3;
	    public static final int SYM_CODE39 = 4;
	    public static final int SYM_CODE49 = 5;         
	    public static final int SYM_CODE93 = 6;
	    public static final int SYM_COMPOSITE = 7;
	    public static final int SYM_DATAMATRIX = 8;
	    public static final int SYM_EAN8 = 9;
	    public static final int SYM_EAN13 = 10;
	    public static final int SYM_INT25 = 11;
	    public static final int SYM_MAXICODE = 12;
	    public static final int SYM_MICROPDF = 13;
	    public static final int SYM_OCR = 14;
	    public static final int SYM_PDF417 = 15;
	    public static final int SYM_POSTNET = 16;
	    public static final int SYM_QR = 17;
	    public static final int SYM_RSS = 18;
	    public static final int SYM_UPCA = 19;
	    public static final int SYM_UPCE0 = 20;
	    public static final int SYM_UPCE1 = 21;
	    public static final int SYM_ISBT = 22;
	    public static final int SYM_BPO = 23;
	    public static final int SYM_CANPOST =24; 
	    public static final int SYM_AUSPOST = 25;
	    public static final int SYM_IATA25 = 26;
	    public static final int SYM_CODABLOCK = 27;
	    public static final int SYM_JAPOST = 28;
	    public static final int SYM_PLANET = 29;
	    public static final int SYM_DUTCHPOST = 30;
	    public static final int SYM_MSI = 31;
	    public static final int SYM_TLCODE39 = 32; 
	    public static final int SYM_TRIOPTIC = 33;
		public static final int SYM_CODE32 = 34;
		public static final int SYM_STRT25 = 35;
		public static final int SYM_MATRIX25 = 36;
	    public static final int SYM_PLESSEY = 37;   
		public static final int SYM_CHINAPOST = 38;
		public static final int SYM_KOREAPOST = 39;
		public static final int SYM_TELEPEN = 40;
	    public static final int SYM_CODE16K = 41;     
	    public static final int SYM_POSICODE = 42;       
		public static final int SYM_COUPONCODE = 43;
	    public static final int SYM_USPS4CB = 44;
		public static final int SYM_IDTAG = 45;
	    public static final int SYM_LABEL = 46;         
		public static final int SYM_GS1_128 = 47;
	    public static final int SYM_HANXIN = 48;
	    public static final int SYM_GRIDMATRIX = 49;    
		public static final int SYM_POSTALS = 50;		
	    public static final int SYM_US_POSTALS1 = 51;   
	    public static final int SYMBOLOGIES = 52;
	    public static final int SYM_ALL = 100;
	}

	public final class SymbologyFlags {
		private SymbologyFlags(){
			
		}
		
		// Flags for use by SymbologyConfig class
		public static final int SYMBOLOGY_ENABLE                = 0x00000001;  // Enable Symbology bit
		public static final int SYMBOLOGY_CHECK_ENABLE          = 0x00000002;  // Enable usage of check character
		public static final int SYMBOLOGY_CHECK_TRANSMIT        = 0x00000004;  // Send check character
		public static final int SYMBOLOGY_START_STOP_XMIT       = 0x00000008;  // Include the start and stop characters in the decoded result string
		public static final int SYMBOLOGY_ENABLE_APPEND_MODE    = 0x00000010;  // Code39 append mode
		public static final int SYMBOLOGY_ENABLE_FULLASCII      = 0x00000020;  // Enable Code39 Full ASCII
		public static final int SYMBOLOGY_NUM_SYS_TRANSMIT      = 0x00000040;  // UPC-A/UPC-e Send Num Sys
		public static final int SYMBOLOGY_2_DIGIT_ADDENDA       = 0x00000080;  // Enable 2 digit Addenda (UPC & EAN)
		public static final int SYMBOLOGY_5_DIGIT_ADDENDA       = 0x00000100;  // Enable 5 digit Addenda (UPC & EAN)
		public static final int SYMBOLOGY_ADDENDA_REQUIRED      = 0x00000200;  // Only allow codes with addenda (UPC & EAN)
		public static final int SYMBOLOGY_ADDENDA_SEPARATOR     = 0x00000400;  // Include Addenda separator space in returned string.
		public static final int SYMBOLOGY_EXPANDED_UPCE         = 0x00000800;  // Extended UPC-E
		public static final int SYMBOLOGY_UPCE1_ENABLE          = 0x00001000;  // UPC-E1 enable (use SYMBOLOGY_ENABLE for UPC-E0)
		public static final int SYMBOLOGY_COMPOSITE_UPC         = 0x00002000;  // Enable UPC composite codes
		public static final int SYMBOLOGY_AUSTRALIAN_BAR_WIDTH  = 0x00010000;  // Include australian postal bar data in string
		public static final int SYMBOLOGY_128_APPEND			= 0x00080000;  // Enable OR dISBALE Code 128 FNC2 append functionality
		public static final int SYMBOLOGY_RSE_ENABLE            = 0x00800000;  // Enable RSS Expanded bit
		public static final int SYMBOLOGY_RSL_ENABLE            = 0x01000000;  // Enable RSS Limited bit
		public static final int SYMBOLOGY_RSS_ENABLE            = 0x02000000;  // Enable RSS bit
		public static final int SYMBOLOGY_RSX_ENABLE_MASK       = 0x03800000;  // Enable all RSS versions
		public static final int SYMBOLOGY_TELEPEN_OLD_STYLE     = 0x04000000;  // Telepen Old Style mode.
		public static final int SYMBOLOGY_POSICODE_LIMITED_1    = 0x08000000;  // PosiCode Limited of 1
		public static final int SYMBOLOGY_POSICODE_LIMITED_2    = 0x10000000;  // PosiCode Limited of 2
	    public static final int SYMBOLOGY_CODABAR_CONCATENATE   = 0x20000000;  // Codabar concatenate.
 
		/* SymbologyConfig class masks */
		public static final int SYM_MASK_FLAGS                  = 0x00000001;  // Flags are valid
		public static final int SYM_MASK_MIN_LEN                = 0x00000002;  // Min Length valid
		public static final int SYM_MASK_MAX_LEN                = 0x00000004;  // Max Length valid
		public static final int SYM_MASK_ALL                    = 0x00000007;  // All fields valid
    
	}
	
	
	public final static class EngineID{
		private EngineID(){
		}
		public static final int UNKNOWN	= -1;
		public static final int NONE=0;
		public static final int IMAGER_4200_ENGINE=1;
		public static final int LASER_SE1200_ENGINE=2;
		public static final int LASER_SE1223_ENGINE=3;
		public static final int IMAGER_IT4000_ENGINE=5;
		public static final int IMAGER_IT4100_ENGINE=6;
		public static final int IMAGER_IT4300_ENGINE=7;
		public static final int IMAGER_IT5100_ENGINE=8;
		public static final int IMAGER_IT5300_ENGINE=9;
		public static final int IMAGER_N5603_ENGINE=12;
		public static final int IMAGER_N5600_ENGINE=13;
	}
	
	
	public final static class EngineType{
		private EngineType(){
		}
		
		public static final int UNKNOWN	= -1;
		public static final int NONE = 0;
		public static final int IMAGER = 1;
		public static final int LASER = 2;
	}

	
	public final static class OCRMode{
		private OCRMode(){
		}
		
		/* Definitions->Properties */
		public static final int OCR_OFF = 0;
		public static final int OCR_NORMAL_VIDEO =1;
		public static final int OCR_INVERSE = 2;
		public static final int OCR_BOTH = 3;
	}		
	

	public final static class OCRTemplate{
		private OCRTemplate(){
		}
	
		public static final int USER = 0x0001;
		public static final int PASSPORT = 0x0002;
		public static final int ISBN = 0x0004;
		public static final int PRICE_FIELD = 0x0008;
		public static final int MICRE13B = 0x0010;
	}		
	
	
	public final static class LightsMode{
		private LightsMode(){
		}
	
		public static final int ILLUM_AIM_OFF = 0; 	/* Neither aimer or illumination */
		public static final int AIMER_ONLY = 1;		/* Aimer only */
		public static final int ILLUM_ONLY = 2;		/* Illumination only */
		public static final int ILLUM_AIM_ON = 3;	/* Aimer and illumination alternating */
		public static final int CONCURRENT = 4;		/* Both aimer and illumination */
	}		
	
	
	public final static class IQImageFormat{
		private IQImageFormat(){
		}
	
		public static final int RAW_BINARY = 0; 	
		public static final int RAW_GRAY = 1;		
	}		
	
	
	public final static class ScanMode{
		private ScanMode(){
		}
		public static final int BATCH = 0;
		public static final int STREAM = 1;
		public static final int CONCURRENT_IQ = 2;
		public static final int CONCURRENT_FAST = 3;
		public static final int CONCURRENT_FAST_IQ = 4;
		public static final int CONFIG_NUMBER = 5;
	}		
	
	public final static class ExposureMode{
		private ExposureMode(){
		}
		public static final int FIXED = 0;
		public static final int ONCHIP = 1;
		public static final int HHP = 2;
		public static final int AUTO_PRESENTATION = 3;
		public static final int CONTEXT_SENSITIVE = 4;
		public static final int OPENLOOP_GAIN = 5;
		public static final int CELLPHONE = 6;
		public static final int AUTO_DUAL_TRACK = 7;
		public static final int CONTEXT_DUAL_TRACK = 8;
		public static final int TEST_PATTERN = 9;
	}		
	
	

	

}
