package com.hsm.barcode;
import android.graphics.Bitmap;
import android.util.Log;

public class Decoder {

  	private int mMayContinueDoDecode = 4;
  	public boolean mayContinue = true;

	private static final String TAG = "Decoder.java";

	  // load the Decoder.SO
	  static {
		try{
			System.loadLibrary("HSMDecoderAPI");
		    Log.d(TAG, "HSMDecoderAPI.so loaded");
		}catch (Exception e) {
			e.printStackTrace();
		}
	  }

		/* Native methods from JNI */
	  	private native int Connect();
	  	private native int Disconnect();
	  	private native String GetErrorMessage(int error);
	  	private native int GetMaxMessageLength();
	  	private native int GetEngineID();
	  	private native int GetPSOCMajorRev();
	  	private native int GetPSOCMinorRev();
	  	private native String GetEngineSerialNumber();
	  	private native int GetEngineType();
		private native String GetAPIRevision();
		private native String GetDecoderRevision();
		private native String GetSecondaryDecoderRevision();
		private native String GetControlLogicRevision();
		private native String GetDecThreadsRevision();
		private native String GetScanDriverRevision();
		private native void GetImagerProperties(ImagerProperties imgProp);
		private native byte[] GetLastImage(ImageAttributes imgAtt);
		private native int GetIQImage(IQImagingProperties IQParams, Bitmap bitmap);
		private native int GetCenteringWindowLimits(CenteringWindowLimits limits);
		private native int SetCenteringWindow(boolean Defaults, CenteringWindow window);
		private native int EnableCenteringWindow(boolean enable);
	  	private native int EnableSymbology(int symID);
	  	private native int DisableSymbology(int symID);
	  	private native int SetSymbologyDefaults(int symID);
	  	private native int GetSymbologyConfig(SymbologyConfig symConfig, boolean Defaults);
	  	private native int SetSymbologyConfig(SymbologyConfig symConfig);
	  	private native int GetSymbologyMinRange(int symID);
	  	private native int GetSymbologyMaxRange(int symID);
	  	private native int SetLightsMode(int Mode);
	  	private native int SetScanMode(int Mode);
	  	private native int SetExposureMode(int Mode);
	  	private native int SetDecodeSearchLimit(int limit);
	  	private native int SetDecodeAttemptLimit(int limit);
	  	private native int SetOCRTemplates(int templates);
	  	private native int SetOCRUserTemplate(int mode, byte[] template);
	  	private native int SetOCRMode(int mode);
	  	private native int WaitForDecode( int dwTimeout);
	  	private native byte[] GetBarcodeByteData();
	  	private native byte GetBarcodeCodeID();
	  	private native byte GetBarcodeAimID();
	  	private native byte GetBarcodeAimModifier();
	  	private native int  GetBarcodeLength();
		public native String GetBarcodeData();
		private native int GetLastDecodeTime();
		private native int WaitForDecodeTwo( int dwTimeout, DecodeResult result);

	    private native int GetSingleFrame(Bitmap bitmap);
	    private native int GetPreviewFrame(Bitmap bitmap);
	    private native int StartScanning();
	    private native int StopScanning();
	    private native int GetImageWidth();
	    private native int GetImageHeight();
	    private native int SetExposureSettings(int[] array);

		private native int SetProperty(int property, int value);
		private native int GetProperty(int property);
		private native String GetStringProperty(int number);




		// Eventually we'll have to throw exceptions for failures but for now we won't
	    /**
	     * This method is used to cancel a decode attempt
	     */
	    public void cancelDecode(){
		    Log.d(TAG, "cancelDecode");
	        mMayContinueDoDecode = 0;
	        mayContinue = false;
	    }


	    /**
	     * The method is used to establish a connection to the decoder library. This method must be called
	     * before any other method
	     */
		public int connectToDecoder(){
		    Log.d(TAG, "connectToDecoder");
			return Connect();
		}


	    /**
	     * The method is used to terminate the connection to the decoder library.
	     */
		public int disconnectFromDecoder (){
		    Log.d(TAG, "disconnectFromDecoder");
			return Disconnect();
		}

		/**
		 *
		 * @param error
		 */
		public String getErrorMessage(int error){
		    Log.d(TAG, "getErrorMEssage");
			return GetErrorMessage(error);
		}

		/**
		 * This method is used to retrieve the maximum size of possible barcode data length
		 * @return - maximum possible size of barcode data that can be returned.
		 */
		public int getMaxMessageLength(){
		    Log.d(TAG, "getMaxMessageLength");
			return GetMaxMessageLength();
		}

		/**
		 * This method returns the PSOC firmware major revision number
		 * @return - the PSOC firmware major revision number
		 */
		public int getPSOCMajorRev(){
		    Log.d(TAG, "getPSOCMajorRev");
			return GetPSOCMajorRev();
		}

		/**
		 * This method returns the PSOC firmware minor revision number
		 * @return - - the PSOC firmware minor revision number
		 */
		public int getPSOCMinorRev(){
		    Log.d(TAG, "getPSOCMinorRev");
			return GetPSOCMinorRev();
		}

		/**
		 * This method returns the the scan engine serial number
		 * @return - engine serial number
		 */
		public String getEngineSerialNumber(){
		    Log.d(TAG, "getEngineSerialNumber");
			return GetEngineSerialNumber();
		}


		/**
		 * This method returns the scan engine's engine ID
		 * @return - engine ID
		 */
		public int getEngineID(){
		    Log.d(TAG, "getEngineID");
			return GetEngineID();
		}

		/**
		 * This method returns an int indicating the scan engine type
		 * 0 = no engine detected
		 * 1 = imager
		 * 2 = laser
		 * @return - engine type
		 */
		public int getEngineType(){
		    Log.d(TAG, "getEngineType");
			return GetEngineType();
		}


		/**
		 * This method returns the API revision
		 * @return - revision string
		 */
		public String getAPIRevision(){
		    Log.d(TAG, "getAPIRevision");
			return GetAPIRevision();

		}

		/**
		 * This method returns the Decoder revision
		 * @return - revision string
		 */
		public String getDecoderRevision(){
		    Log.d(TAG, "getDecoderRevision");
			return GetDecoderRevision();

		}

		/**
		 * This method returns the Secondary Decoder revision
		 * @return - revision string
		 */
		public String getSecondaryDecoderRevision(){
		    Log.d(TAG, "getSecondaryDecoderRevision");
			return GetSecondaryDecoderRevision();

		}

		/**
		 * This method returns the Control Logic (DCL) revision
		 * @return - revision string
		 */
		public String getControlLogicRevision(){
		    Log.d(TAG, "getControlLogicRevision");
			return GetControlLogicRevision();

		}

		/**
		 * This method returns the DecThreads revision
		 * @return - revision string
		 */
		public String getDecThreadsRevision(){
		    Log.d(TAG, "getDecThreadsRevision");
			return GetDecThreadsRevision();

		}

		/**
		 * This method returns the Scan Driver revision
		 * @return - revision string
		 */
		public String getScanDriverRevision(){
		    Log.d(TAG, "getScanDriverRevision");
			return GetScanDriverRevision();

		}

		/**
		 * This method is used to enable a barcode symbol type
		 */
		public int enableSymbology(int symbologyID){
		    Log.d(TAG, "enableBarcode");
		    return EnableSymbology(symbologyID);
		}

		/**
		 * This method is used to disable a barcode symbol type
		 */
		public int disableSymbology(int symbologyID){
		    Log.d(TAG, "disableBarcode");
		    return DisableSymbology(symbologyID);
		}

		/**
		 * This method is used to reset a barcode symbol type to its default condition
		 */
		public int setSymbologyDefaults(int symbologyID){
		    Log.d(TAG, "setSymbologyDefaults");
		    return DisableSymbology(symbologyID);
		}

		/**
		 * This method is used to acquire current symbology configuration
		 */
		public int getSymbologyConfig(SymbologyConfig symConfig, boolean DefaultValues){
		    Log.d(TAG, "getSymbologyConfig");
		    return GetSymbologyConfig(symConfig, DefaultValues);
		}

		/**
		 * This method is used to setup current symbology configuration
		 */
		public int setSymbologyConfig(SymbologyConfig symConfig){
		    Log.d(TAG, "setSymbologyConfig");
		    return SetSymbologyConfig(symConfig);
		}

		/**
		 * This method is used to acquire current symbology minimum possible length
		 * @return range value integer
		 */
		public int getSymbologyMinRange(int symbologyID){
			int min;
		    Log.d(TAG, "GetSymbologyMinRange");
			min = GetSymbologyMinRange(symbologyID);
			return min;
		}

		/**
		 * This method is used to acquire current symbology maximum possible length
		 * @return range value integer
		 */
		public int getSymbologyMaxRange(int symbologyID){
			int min;
		    Log.d(TAG, "GetSymbologyMinRange");
			min = GetSymbologyMaxRange(symbologyID);
			return min;
		}

		/**
		 * This method is used to acquire current imager properties
		 */
		public void getImagerProperties(ImagerProperties imgProp)
		{
			Log.d(TAG, "getImagerProperties");
			GetImagerProperties(imgProp);
		}

		/**
		 * This method is used to setup the illumination/aimer mode
		 * LM_ILLUM_AIMER_OFF=0, // Neither aimers or illumination used during scanning
		 * LM_AIMER_ONLY=1, // Aimer only used during scanning
		 * LM_ILLUM_ONLY=2, // Illumination LEDs only used during scanning
		 * LM_ILLUM_AIMER=3, // Alternating aimer & illumination during scanning (default)
		 */
		public int setLightsMode(int Mode){
			Log.d(TAG, "setLightsMode");
			return SetLightsMode(Mode);
		}

		/**
		 * This method is used to setup the scan mode
		 */
		public int setScanMode(int Mode){
			Log.d(TAG, "setScanMode");
			return SetScanMode(Mode);
		}

		/**
		 * This method is used to setup the exposure mode
		 * EXP_MODE_FIXED=0,not recommended
		 * EXP_MODE_AUTO=2
		 */
		public int setExposureMode(int Mode){
			Log.d(TAG, "setExposureMode");
			return SetExposureMode(Mode);
		}

		/**
		 * This method is used to setup the decoder search time limit
		 */
		public void setDecodeSearchLimit(int limit){
			Log.d(TAG, "setDecodeSearchLimit");
			SetDecodeSearchLimit(limit);
			return;
		}

		/**
		 * This method is used to setup the decoder decode time limit
		 */
		public int setDecodeAttemptLimit(int limit){
			Log.d(TAG, "setDecodeAttemptLimit");
			return SetDecodeAttemptLimit(limit);
		}

		/**
		 * This method is used to acquire the centering window coordinates
		 */
		public int getCenteringWindowLimits(CenteringWindowLimits limits){
			Log.d(TAG, "GetCenteringWindowLimits");
			return GetCenteringWindowLimits(limits);
		}

		/**
		 * This method is used to setup the centering window coordinates
		 */
		public int setDecodeCenteringWindow(CenteringWindow window){
			Log.d(TAG, "setDecodeCenteringWindow");
			return SetCenteringWindow(false, window);
		}

		/**
		 * This method is used to enable/disable the centering window feature
		 */
		public int enableDecodeCenteringWindow(boolean enable){
			Log.d(TAG, "EnableDecodeCenteringWindow");
			return EnableCenteringWindow(enable);
		}


		/**
		 * This method is used to retrieve the last image acquired by the scanner
		 * @return raw image pixels in a byte array
		 */
		public byte[] getLastImage(ImageAttributes imgAtt){
		    Log.d(TAG, "getLastImage");
			return(GetLastImage(imgAtt));

		}

		/**
		 * This method is used to retrieve the data from the last successful decode in byte format
		 * @return barcode data in a byte array
		 */
		public byte[] getBarcodeByteData(){
		    Log.d(TAG, "getBarcodeByteData");
			return(GetBarcodeByteData());

		}

		/**
		 * This method is used to retrieve all barcode information for each barcode decoded from an image with multiple barcodes
		 * This requires multiple callbacks - 1 for each barcode decoded.
		 * @return boolean - success
		 */
		public boolean MultiReadCallback(byte codeID, byte aimID, byte AimMod, int length, byte[] data){
		    Log.d(TAG, "MultiReadCallback enter");
    		Log.d(TAG, "codeID = " + codeID);
    		Log.d(TAG, "aimID = " + aimID);
    		Log.d(TAG, "AimMod = " + AimMod);
    		Log.d(TAG, "length = " + length);
    		String value = new String(data);
     		Log.d(TAG, value);
		    return true;
		}

		/**
		 * This method will capture a bitmap image of the specified Intelligent Image
		 * @return int
		 */
		public int getIQImage(IQImagingProperties IQParams, Bitmap bitmap){
		    Log.d(TAG, "getIQImage");
			return(GetIQImage(IQParams, bitmap));

		}


		/**
		 * This method is used to set multiple user template definitions for decoding OCR images
		 */
		public int setOCRTemplates(int templates){
			Log.d(TAG, "setOCRTemplates");
			return SetOCRTemplates(templates);
		}

		/**
		 * This method is used to set a user template definition for decoding OCR images
		 */
		public int setOCRUserTemplate(int mode, byte[] template){
			Log.d(TAG, "setOCRUserTemplate");
			return SetOCRUserTemplate(mode, template);
		}

		/**
		 * This method set the desired OCR decode mode.  Refer to the OEM API OCR User�s Guide for more information.
		 */
		public int setOCRMode(int mode){
			Log.d(TAG, "setOCRMode");
			return SetOCRMode(mode);
		}

		/**
		 * This method is used to start a blocking decode attempt by the scan engine.
		 *  This function does not return to the calling application until either a symbol is decoded, the time out period has elapsed,
		 *  or the optional callback function returns a value of false.
		 */
		public int waitForDecode(int timeOut){
		    Log.d(TAG, "waitForDecode");
		    mMayContinueDoDecode = 4;
		    return WaitForDecode(timeOut);
		}

		/**
		 * This method is used to retrieve the CodeID of the last barcode decoded.
		 * @return byte value of the codeID
		 */
		public byte getCodeID(){
		    Log.d(TAG, "getCodeID");
		    return GetBarcodeCodeID();
		}

		/**
		 * This method is used to retrieve the AimID of the last barcode decoded.
		 * @return byte value of the AimID
		 */
		public byte getAimID(){
		    Log.d(TAG, "getAimID");
			return GetBarcodeAimID();
		}

		/**
		 * This method is used to retrieve the Aim modifier of the last barcode decoded.
		 * @return byte value of the Aim modifier
		 */
		public byte getAimModifier(){
		    Log.d(TAG, "getAimModifier");
			return GetBarcodeAimModifier();
		}

		/**
		 * This method is used to retrieve the length of the last barcode decoded.
		 * @return integer value of the length
		 */
		public int getLength(){
		    Log.d(TAG, "getLength");
			return GetBarcodeLength();
		}

		/**
		 * This method is used to retrieve the data contained in the last barcode decoded. Should not be used for barcodes containing NULLs or other non-ASCII characters.
		 * @return string data of barcode
		 */
		public String getDecodeData(){
		    Log.d(TAG, "getDecodeData");
			return GetBarcodeData();
		}

		/**
		 * This method is used to retrieve the duration to decode the last barcode decoded.
		 * @return integer value of the time
		 */
		public int getLastDecodeTime(){
		    Log.d(TAG, "getLastDecodeTime");
			return GetLastDecodeTime();
		}


		public int waitForDecodeTwo(int timeOut, DecodeResult result){
			Log.d(TAG, "waitForDecode2 enter");
			mMayContinueDoDecode = 4;
			int RetVal = WaitForDecodeTwo(timeOut, result);
		    Log.d(TAG, "waitForDecode2 exit");

			return RetVal;
		}

		private int KeepGoing(){
		    Log.d(TAG, "KeepGoing");
			return 1;
		}

		// For imaging
		public int getSingleFrame(Bitmap bitmap){
		    Log.d(TAG, "getSingleFrame");
			return(GetSingleFrame(bitmap));

		}

		// 1/4 resolution image
		public int getPreviewFrame(Bitmap bitmap){
		    Log.d(TAG, "getPreviewFrame");
			return(GetPreviewFrame(bitmap));

		}

	    /**
	     * The method causes the image engine to start continuous collecting of images.
		 * @return integer - success
	     */
		public int startScanning(){
		    Log.d(TAG, "startScanning");
			return StartScanning();
		}

	    /**
	     * The method causes the image engine to stop continuous collecting of images.
		 * @return integer - success
	     */
		public int stopScanning(){
		    Log.d(TAG, "stopScanning");
			return StopScanning();
		}

	    /**
	     * The method is used to retrieve the width (number of pixels) of the last captured image.
	     * This information can be used to calculate the image size.
		 * @return integer value of the width
	     */
		public int getImageWidth(){
		    Log.d(TAG, "getImageWidth");
			return GetImageWidth();
		}

	    /**
	     * The method is used to retrieve the height (number of pixels) of the last captured image.
	     * This information can be used to calculate the image size.
		 * @return integer value of the height
	     */
		public int getImageHeight(){
		    Log.d(TAG, "getImageHeight");
			return GetImageHeight();
		}

		public int setExposueSettings(int[] expSettings){
			return SetExposureSettings(expSettings);
		}

	    /**
	     * The method is used to configure the decoder by configuring specific decoder properties.
	     */
		public int setProperty(int property, int value){
				return SetProperty(property, value);
		}

	    /**
	     * The method is used to retrieve specific decoder properties in the decoder configuration.
	     */
		public int getProperty(int property){
				return GetProperty(property);
		}


}
