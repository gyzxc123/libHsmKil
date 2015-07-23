package com.android.oemjartest;

import com.hsm.barcode.*;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.app.Activity;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;
import android.view.KeyEvent;

import java.io.FileInputStream;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.IntBuffer;
import java.util.Date;
//import java.awt.*;

/**
 * @author E412227
 *
 */
public class woohoo extends Activity {
	TextView  tv;
	Decoder decdr = null;
	int CBRENA = 0x1a01f001;
	int CBRCONCAT = 0x1a01f007;
	int TEMPLATE = 0x9a02d002;
	int DEC_ECI_HANDLING = 0x1B001006;

	boolean maxiEnabled = false;
	boolean okayToDecode = true;
	boolean okayToSaveRaw = true;
	boolean okayToSavePng = true;
    private int rawPicCount = 0;
    private int pngPicCount = 0;
	boolean oktodisconnect = true;
   	private static final String TAG = "Test_app";
    private int delay = 4000;
    private int searchLimit = 800;
    private int decodeLimit = 800;
    private IQImagingProperties IQ = new IQImagingProperties();
	private Bitmap mIQBitMap;
	byte imageIn[] = new byte[1100*800];
    byte[] barcodeData = new byte[8192];

	int expSettings[] = {ExposureValues.Setting.MaximumExposure, 20,
						ExposureValues.Setting.AutoExposureMode, 1,
						ExposureValues.Setting.MaximumGain, 200};

	public MediaPlayer mediaPlayer;
	private OnCompletionListener beepListener = new BeepListener();
	private static final float BEEP_VOLUME = 0.10f;

	public static final int IQ_WIDTH = 120;

	public static final int IQ_HEIGHT = 60;
	public static final int IQ_RESOLUTION = 4;
	int aimeron = 0;

	Object syncToken;
	// Thread decodeThread;
	DecodeThread oDecodeThread;


   public class DecodeThread extends Thread{

        Object syncToken;
    	boolean Decoding = false;
    	DecodeResult decRes = new DecodeResult();

        public DecodeThread(Object syncToken)
        {
            this.syncToken = syncToken;
        }

        public void run()
        {
        	// need to add a clean way to end exit the thread....
            while(true)
            {
                synchronized (syncToken)
                {
                    try {
                        syncToken.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

           	    Log. d(TAG, "thread unblocked");
           	    try{
           	    	Decoding = true;
					 while(Decoding) {
	           	    	decdr.waitForDecodeTwo(delay, decRes);
	           	    	if (decRes.length > 0){
	           	    		Log. d(TAG, "decode success");
	           	    		Decoding = false;
		           	    	// Post message for screen update
		           	    	threadHandler.sendEmptyMessage(1);

	           	    	}
	           	    	else{
	           	    		Log. d(TAG, "NO decode");
	           	    		Decoding = false;
	           	    	}

 						sleep(100);
					}

           	    	Log. d(TAG, "back to waiting");
           	    }
           	    catch(InterruptedException ex) {
           	    	ex.printStackTrace();
           	    }
            }
        }

        // Used to stop the preview thread loop
        public void setLoopFlag(boolean state)
        {
        	Decoding = state;
        }

   }

	// Receives decode thread's messages & drives screen refresh
	private Handler threadHandler = new Handler() {
	    public void handleMessage(android.os.Message msg) {
   	    	Log. d(TAG, "handleMessage");
	    }
	};


	void TestMethods()
	{

       int expResult = decdr.setExposueSettings(expSettings);


   	    // test the various get functions
   	    String msg = decdr.getErrorMessage(7);
   	    int maxSize = decdr.getMaxMessageLength();
   	    int engineID = decdr.getEngineID();
   	    int engineType = decdr.getEngineType();

 	    ImagerProperties imgProp = new ImagerProperties();
  	    decdr.getImagerProperties(imgProp);
  	    int major = decdr.getPSOCMajorRev();
  	    int minor = decdr.getPSOCMinorRev();
  	    String serialnum = decdr.getEngineSerialNumber();

   	    String aa = decdr.getAPIRevision();
   	    String bb = decdr.getDecoderRevision();
   	    String cc = decdr.getSecondaryDecoderRevision();
   	    String kk = decdr.getDecThreadsRevision();
   	    String jj = decdr.getControlLogicRevision();
   	    String mm = decdr.getScanDriverRevision();

   	    // Now verify the symbology config functions
   	    // by changing some Code 39 config settings
  	    int minLimit = decdr.getSymbologyMinRange(DecoderConfigValues.SymbologyID.SYM_CODE39);
  	    int maxLimit = decdr.getSymbologyMaxRange(DecoderConfigValues.SymbologyID.SYM_CODE39);
   	    SymbologyConfig c39config = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_CODE39);
   	    decdr.getSymbologyConfig(c39config,false);

   	    c39config.MinLength = 8;
   	    c39config.MaxLength = 40;
   	    c39config.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
   	    c39config.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE | DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE_FULLASCII;
  	    decdr.setSymbologyConfig(c39config);

 	    // Now read back the test values set earlier
   	    // again checking if we can read defaults and current
   	    c39config.MinLength = 0;
   	    c39config.MaxLength = 0;
   	    c39config.Mask = 0;
   	    c39config.Flags = 0;
   	    decdr.getSymbologyConfig(c39config, false);
 	    decdr.getSymbologyConfig(c39config, true);


	}


    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {

        byte[] OCRUser = new byte[]{0x01,0x03,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x00};
    	byte[] OCRtest = new byte[]{0x01,0x03,0x08,0x0F,0x01,0x32,0x00};
    	super.onCreate(savedInstanceState);

        syncToken = new Object();
        oDecodeThread = new DecodeThread(syncToken);
        oDecodeThread.start();

        //setContentView(R.layout.main);
        String revInfo = "blah";

	    //Log. d(TAG, getString(R.string.initializing_decoder_));
        if (decdr == null)
        {
        	decdr = new Decoder();

	        // This will read an image from an SD card into memory so we can test the DecodeImage method
	        ReadImage();

		    Log. d(TAG, "Connecting to engine...");
	        decdr.connectToDecoder();
	        //TestMethods();

	        // Retrieve the various revision for display
	   	    revInfo = "\n " + decdr.getAPIRevision()  + "\n ";
	   	    revInfo += decdr.getDecoderRevision();
	   	    revInfo += decdr.getSecondaryDecoderRevision();
	   	    String kk = decdr.getDecThreadsRevision();
	   	    String jj = decdr.getControlLogicRevision();

	   	    //decdr.setLightsMode(7);
        }

 	    ImagerProperties imgProp = new ImagerProperties();
  	    decdr.getImagerProperties(imgProp);
  	    int major = decdr.getPSOCMajorRev();
  	    int minor = decdr.getPSOCMinorRev();
  	    String serialnum = decdr.getEngineSerialNumber();
  	    String partnum = imgProp.EnginePartNum;


   	    // do a sample UPC configure
   	    SymbologyConfig upcConfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_UPCA);
  	    //decdr.getSymbologyConfig(upcConfig,false);
  	    //upcConfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
  	    //upcConfig.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_NUM_SYS_TRANSMIT |
  	    //				  DecoderConfigValues.SymbologyFlags.SYMBOLOGY_2_DIGIT_ADDENDA |
  	    //				  DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ADDENDA_REQUIRED;
  	    //decdr.setSymbologyConfig(upcConfig);

   	    //SymbologyConfig eanConfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_EAN8);
  	    //decdr.getSymbologyConfig(eanConfig,false);
  	    //eanConfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
  	    //eanConfig.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_NUM_SYS_TRANSMIT |
  	    //				  DecoderConfigValues.SymbologyFlags.SYMBOLOGY_2_DIGIT_ADDENDA |
  	    //				  DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE |
  	    //				  DecoderConfigValues.SymbologyFlags.SYMBOLOGY_5_DIGIT_ADDENDA;
  	    //decdr.setSymbologyConfig(eanConfig);
  	    // make sure we can read both the default and current settings.
  	    //decdr.getSymbologyConfig(eanConfig,true);	// default
  	    //decdr.getSymbologyConfig(eanConfig,false);	// current


   	    //SymbologyConfig AZconfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_AZTEC);
   	    //decdr.getSymbologyConfig(AZconfig,false);
   	    //AZconfig.MinLength = 1;
   	    //AZconfig.MaxLength = 3832;
   	    //AZconfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
   	    //AZconfig.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE;
  	    //decdr.setSymbologyConfig(AZconfig);


  	    // Read the current RSS config settings
   	    //SymbologyConfig rss = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_RSS);
   	    //decdr.getSymbologyConfig(rss,false);
   	    //rss.MinLength = 2;
   	    //rss.MaxLength = 10;
   	    //rss.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
   	    //rss.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_RSE_ENABLE;
  	    //decdr.setSymbologyConfig(rss);
  	    //decdr.getSymbologyConfig(rss,true);
  	    //decdr.getSymbologyConfig(rss,false);
  	    //decdr.getSymbologyConfig(rss,false);


   	    // Read the current Codabar configuration settings
   	    //SymbologyConfig C11config = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_CODE11);
   	    //decdr.getSymbologyConfig(C11config,false);
   	    //C11config.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_FLAGS;
   	    //C11config.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE;
   	    //C11config.Flags |= DecoderConfigValues.SymbologyFlags.SYMBOLOGY_CHECK_ENABLE;
 	    //decdr.setSymbologyConfig(C11config);


   	    //int cben = decdr.getProperty(CBRENA);
   	    //int cbcon = decdr.getProperty(CBRCONCAT);
	    //decdr.setScanMode(DecoderConfigValues.ScanMode.CONCURRENT_FAST_IQ);


  	    // misc functions
   	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_DATAMATRIX);
   	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_CANPOST);
  	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_KOREAPOST);


   	    //SymbologyConfig cbrConfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_CODABAR);
   	    //decdr.getSymbologyConfig(cbrConfig,false);

   	    // Enable append
   	    //cbrConfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
   	    //cbrConfig.Flags |= DecoderConfigValues.SymbologyFlags.SYMBOLOGY_CODABAR_CONCATENATE;
   	    //decdr.setSymbologyConfig(cbrConfig);

   	    //decdr.setLightsMode(DecoderConfigValues.LightsMode.ILLUM_AIM_ON);
   	    //decdr.setOCRTemplates(DecoderConfigValues.OCRTemplate.USER);
   	    //decdr.setOCRUserTemplate(DecoderConfigValues.OCRMode.OCR_OFF, OCRUser);
   	    //decdr.setOCRMode(DecoderConfigValues.OCRMode.OCR_NORMAL_VIDEO);

    	    // the time limits are not typically set, done here for testing only
   	    //decdr.setDecodeSearchLimit(searchLimit);
   	    //decdr.setDecodeAttemptLimit(decodeLimit);

   	    // again, this won't work until the lower layers are implemented
   	    //decdr.aimerOn(false);
   	    //decdr.lightsOn(false);


   	    //CenteringWindowLimits cwl = new CenteringWindowLimits();
   	    //decdr.getCenteringWindowLimits(cwl);

   	    CenteringWindow cw = new CenteringWindow();
   	    cw.LowerRightX = 421;
   	    cw.LowerRightY = 325;
   	    cw.UpperLeftX = 411;
   	    cw.UpperLeftY = 315;
//FauthD centring failed
//   	    decdr.setDecodeCenteringWindow(cw);
//   	    decdr.enableDecodeCenteringWindow(true);
//FauthD

   	    //decdr.disableSymbology(DecoderConfigValues.SymbologyID.SYM_POSTALS);
   	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_COUPONCODE);


   	    // Enable append
	    //SymbologyConfig upcConfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_UPCA);
   	    //decdr.getSymbologyConfig(upcConfig,false);
   	    //upcConfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_FLAGS;
   	    //upcConfig.Flags |= DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE;
   	    //upcConfig.Flags |= DecoderConfigValues.SymbologyFlags.SYMBOLOGY_NUM_SYS_TRANSMIT;
   	    //decdr.setSymbologyConfig(upcConfig);

 	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_UPCA);
    	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_COUPONCODE);

  	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_ALL);

   	    //int minss = decdr.getSymbologyMinRange(DecoderConfigValues.SymbologyID.SYM_COMPOSITE);
  	    //int maxss = decdr.getSymbologyMaxRange(DecoderConfigValues.SymbologyID.SYM_COMPOSITE);

  	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_RSS);
   	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_COMPOSITE);
	    //SymbologyConfig comConfig = new SymbologyConfig(DecoderConfigValues.SymbologyID.SYM_COMPOSITE);
	    //comConfig.Flags = DecoderConfigValues.SymbologyFlags.SYMBOLOGY_ENABLE;
	    //comConfig.Mask = DecoderConfigValues.SymbologyFlags.SYM_MASK_ALL;
	    //comConfig.MinLength = 1;
	    //comConfig.MaxLength = 4;
	    //decdr.setSymbologyConfig(comConfig);
	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_GS1_128);
	    //decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_DATAMATRIX);

   	    //decdr.setProperty(DEC_ECI_HANDLING, 2);

  	    initSound();
  	    revInfo += "\n Press SCAN key to scan";
	    tv = new TextView(this);
        tv.setText( revInfo );
        setContentView(tv);

        }


    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	String text = "onKeyDown enter: ";
    	text += Integer.toString(keyCode);
    	Log. d(TAG, text);
//    	Log. d(TAG, "onKeyDown enter");
    	switch (keyCode) {
    	case 92:
    	case 292:
    	case 0:
    	case 66: // the orange key underneath the display for Gen2Wave
    		if (okayToDecode == true){
    			okayToDecode = false;
        		scanBarcode();
//                synchronized(syncToken)
//                {
//                    Log. d(TAG, "signaling decode thread");
//                    syncToken.notify();
//               }

    		}
    		return true;
    	case KeyEvent.KEYCODE_G:
    		if (okayToSaveRaw){
    			okayToSaveRaw = false;
    			saveLastImageRaw();
    		}
    		return true;
    	case KeyEvent.KEYCODE_J:
    		if (okayToSavePng){
    			okayToSavePng = false;
    			saveLastImagePng();
    		}
    		return true;
    	case KeyEvent.KEYCODE_BACK:
    		this.finish();
    		return true;

    	case KeyEvent.KEYCODE_A:
    		if (aimeron == 1){
    			decdr.stopScanning();
    			decdr.setLightsMode(5);
    			aimeron = 0;
    		}
    		else{
    			decdr.setLightsMode(1);
    			decdr.startScanning();
    			aimeron = 1;
    		}
    		return true;

    	case KeyEvent.KEYCODE_S:
//  		if (okayToDecode == true){
//  			okayToDecode = false;
//  			decdr.waitMultipleDecode(delay);
//  		}
			return true;

    	case KeyEvent.KEYCODE_D:
    		if (oktodisconnect)
    		{
    			oktodisconnect = false;
    			decdr.disconnectFromDecoder();
    		}
  			return true;
    	case KeyEvent.KEYCODE_M:
    		if (maxiEnabled){
    			maxiEnabled = false;
    			decdr.disableSymbology(DecoderConfigValues.SymbologyID.SYM_MAXICODE);
   	    		Log. d(TAG, "Maxicode disabled");
    		}
    		else{
    			maxiEnabled = true;
    			decdr.enableSymbology(DecoderConfigValues.SymbologyID.SYM_MAXICODE);
   	    		Log. d(TAG, "Maxicode enabled");
    		}
  			return true;
   	default:
    		return false;
    	}
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
	    //Log. d(TAG, "onKeyUp enter");
    	switch (keyCode) {
    	case KeyEvent.KEYCODE_ENTER:
    	case 92:
    	case 292:
    	case 0:
 			oDecodeThread.setLoopFlag(false);
    		decdr.cancelDecode();
    		okayToDecode = true;
    		return true;
    	case KeyEvent.KEYCODE_G:
    		okayToSaveRaw = true;
    		return true;
       	case KeyEvent.KEYCODE_J:
    		okayToSavePng = true;
    		return true;
    	case KeyEvent.KEYCODE_S:
    		decdr.cancelDecode();
    		okayToDecode = true;
			return true;

       	case KeyEvent.KEYCODE_BACK:
    		this.finish();
    		return true;
    	default:
    		return false;
    	}

    }

    private void saveLastImageRaw()
    {
    	ImageAttributes attr = new ImageAttributes();
    	byte[] image;
   	    image = decdr.getLastImage(attr);
   	    int width = decdr.getImageWidth();
   	    int height = decdr.getImageHeight();
   	    StoreByteImage(image, 100, "raw");
    }

    private void saveLastImagePng()
    {
    	ImageAttributes attr = new ImageAttributes();
    	byte[] image;
   	    image = decdr.getLastImage(attr);
   	    StoreByteImage(image, 100, "png");
    }

    private void scanBarcode(){

    	DecodeResult decRes = new DecodeResult();
   	    String bcdata = "blah";


   	    /*
   	    decdr.waitForDecode(delay);
   	    int length = decdr.getLength();
		Log.d(TAG, "decode data length " + length);
   	    if (length > 0){
   	    	playSound();
	   	    bcdata = "\n Data = " + decdr.getDecodeData() + "\n";
	   	    bcdata += " Length = " + Integer.toString(length) + "\n";
	   	    bcdata += " Code ID = " + (char) decdr.getCodeID() + "\n";
	   	    bcdata += " AIM ID = " + (char) decdr.getAimID() + "\n";
	   	    bcdata += " AIM Modifier = " + (char) decdr.getAimModifier() + "\n";
	    }
	    else{
	   	    bcdata = "\n No decode";
	    }

   	    */

    	decdr.waitForDecodeTwo(delay,decRes);

   	    if (decRes.length != 0){
	   	    	playSound();
		   	    int lastTime = decdr.getLastDecodeTime();
		   	    bcdata = "\n Data = " + decRes.barcodeData + "\n";
		   	    bcdata += " Length = " + Integer.toString(decRes.length) + "\n";
		   	    bcdata += " Code ID = " + (char) decRes.codeId + "\n";
		   	    bcdata += " AIM ID = " + (char) decRes.aimId + "\n";
		   	    bcdata += " AIM Modifier = " + (char) decRes.aimModifier + "\n";
		   	    bcdata += " Decode time = " + lastTime + "\n";
		   	    barcodeData = decdr.getBarcodeByteData();
		   	    int jj = 5;

  	    }
   	    else{
   	   	    bcdata = "\n No decode";
   	    }

       	tv.setText(bcdata);

    }


    /**
     * Creates the beep MediaPlayer in advance so that the sound can be triggered with the least
     * latency possible.
     */
    private void initSound() {
      if (mediaPlayer == null) {
        // The volume on STREAM_SYSTEM is not adjustable, and users found it too loud,
        // so we now play on the music stream.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        mediaPlayer = new MediaPlayer();
        mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        mediaPlayer.setOnCompletionListener(beepListener);
        AssetFileDescriptor file = getResources().openRawResourceFd(R.raw.beep);
        try {
          mediaPlayer.setDataSource(file.getFileDescriptor(), file.getStartOffset(),
              file.getLength());
          file.close();
          mediaPlayer.setVolume(BEEP_VOLUME, BEEP_VOLUME);
          mediaPlayer.prepare();
        } catch (IOException e) {
          mediaPlayer = null;
        }
      }
    }


    /* play the beep */
    private void playSound() {
      if (mediaPlayer != null) {
        mediaPlayer.start();
      }
    }


    /*When the beep has finished playing, rewind to queue up another one. */
    private static class BeepListener implements OnCompletionListener {
      public void onCompletion(MediaPlayer mediaPlayer) {
        mediaPlayer.seekTo(0);
      }
    }

  public void ReadImage()
  {
	  String path = Environment.getExternalStorageDirectory().getAbsolutePath();
	  //path += "/Image2_640x480.raw";
	  path += "/Image1_832x640.raw";
	  //path += "/Image2_752x480.raw";
	  //path += "/Image1_1024x768.raw";
	  //path += "/Maxicode.raw";			// the maxicode.raw is the clothe that caused failures
	  File imgFile = new File(path);
	  try
	  {
		  FileInputStream fis = new FileInputStream(imgFile);
//		  byte imageIn[] = new byte[(int)imgFile.length()];
		  int length = (int)imgFile.length();
		  fis.read(imageIn);
		  fis.close();
		  Log.d(TAG, path + " loaded");

	  }
	  catch (FileNotFoundException e)
	  {
		  Log.d(TAG, "Image 1 not found for input");
	  }
	  catch(IOException ioe)
	  {
		  Log.d(TAG, "Image 1 read fail");
	  }
   }
	public boolean StoreByteImage(byte[] imageData,	int quality, String expName) {

	    File sdImageMainDirectory = new File("/sdcard/Test");
	    File imageFilePath =  new File(sdImageMainDirectory.getPath() + "/Images/");

	    imageFilePath.mkdirs();
		FileOutputStream fileOutputStream1 = null;
		BufferedOutputStream bos = null;
		String nameFile = "";
		try {
			BitmapFactory.Options options=new BitmapFactory.Options();
			options.inSampleSize = 1;

			int width = 0;
			int height = 0;
			Bitmap myImage = null;
			if(expName == "raw"){
				width = 832;
				height = 640;
				//Deal with the Delivery task if it is running
				nameFile = imageFilePath.toString() +"/" + "RawImage_"+rawPicCount+".raw";
				fileOutputStream1 = new FileOutputStream(nameFile);
				bos = new BufferedOutputStream(fileOutputStream1);
				bos.write(imageData);
				Log.d(TAG, "RAW file save success");
				Toast.makeText(getApplicationContext(), "RawImage_"+rawPicCount+".raw saved", Toast.LENGTH_LONG).show();
				rawPicCount++;
			}
			else if(expName == "png"){
				width = 832;
				height = 640;
				myImage = renderCroppedGreyscaleBitmap(imageData, 0, 0, width, height, width);
				Log.d(TAG, "imageData length " + imageData.length);
				Log.d(TAG, "Preview width: " + width +
							" Preview height: " + height);
				nameFile = "Image_";

				if(myImage.isMutable())
					Log.d(TAG, "Image is mutable");

				String myNameFile = imageFilePath.toString() +"/" + nameFile + pngPicCount + ".png";
				fileOutputStream1 = new FileOutputStream(myNameFile);
				bos = new BufferedOutputStream(fileOutputStream1);
				myImage.compress(Bitmap.CompressFormat.PNG, quality, bos);
				Log.d(TAG, "PNG file save success");
				Toast.makeText(getApplicationContext(), "Image_"+pngPicCount+".png saved", Toast.LENGTH_LONG).show();
				pngPicCount++;
			}
			bos.flush();
			bos.close();

		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			Log.d(TAG,"Image save was unsuccessful");
			e.printStackTrace();
		}
		return true;
	}

	/*
	 * Can create a cropped greyscale image of ONLY preview images
	 */
	public Bitmap renderCroppedGreyscaleBitmap(byte[] data, int top, int left, int width, int height, int dataWidth) {

	    int imageWidth = dataWidth;
	    int[] pixels = new int[width * height];
	    byte[] yuv = data;
	    int inputOffset = top * imageWidth + left;

	    for (int y = 0; y < height; y++) {
	        int outputOffset = y * width;
	        for (int x = 0; x < width; x++) {
	          int grey = yuv[inputOffset + x] & 0xff;
	          pixels[outputOffset + x] = (0xff000000) | (grey * 0x00010101);
	        }
	        inputOffset += imageWidth;
	      }

	      Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8);
	      bitmap.setPixels(pixels, 0, width, 0, 0, width, height);
	      return bitmap;
	  }


	public boolean StoreIQImage() {

	    File sdImageMainDirectory = new File("/sdcard/Test");
	    File imageFilePath =  new File(sdImageMainDirectory.getPath() + "/Images/");

	    imageFilePath.mkdirs();
		FileOutputStream fileOutputStream1 = null;
		BufferedOutputStream bos = null;
		String nameFile = "";
		try {
			BitmapFactory.Options options=new BitmapFactory.Options();
			options.inSampleSize = 1;

			nameFile = "IQImage_";

			if(mIQBitMap.isMutable())
				Log.d(TAG, "Image is mutable");

			String myNameFile = imageFilePath.toString() +"/" + nameFile + pngPicCount + ".png";
			fileOutputStream1 = new FileOutputStream(myNameFile);
			bos = new BufferedOutputStream(fileOutputStream1);
			if (mIQBitMap.compress(Bitmap.CompressFormat.PNG, 100, bos) == true){
				Log.d(TAG, "PNG file save success");
				Toast.makeText(getApplicationContext(), "IQImage_"+pngPicCount+".png saved", Toast.LENGTH_LONG).show();
			}
			else{
				Log.d(TAG, "PNG file save FAIL");
				Toast.makeText(getApplicationContext(), "IQImage save failed", Toast.LENGTH_LONG).show();

			}
			pngPicCount++;
			bos.flush();
			bos.close();

		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			Log.d(TAG,"Image save was unsuccessful");
			e.printStackTrace();
		}
		return true;
	}

	public void saveBitmap() {
		String path = "/sdcard";
		String image_name = "image " + (new Date().getTime()) + " .png";

		OutputStream outStream = null;
		File file = new File(path, "/"+ image_name);

		try {
			outStream = new FileOutputStream(file);
			mIQBitMap.compress(Bitmap.CompressFormat.PNG, 100, outStream);
			outStream.flush();
			outStream.close();

		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}


	private static int cycle = 0;

	public void createBitmap(int width, int height){

		//GetSingleFrame(mBitmap);

		// the following code is only for test
		// we should call APIs from low level so to create bitmap


		int[] color = new int[width * height];

		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
			{
				int which = (y ^ x) & 16;
				if (which != 0) {
					//mBitmap.setPixel(x, y, Color.rgb(cycle, cycle, cycle));
					color[ y * width + x] = Color.rgb(cycle, cycle, cycle);
				}
				else {
					//mBitmap.setPixel(x, y, Color.rgb(255 - cycle, 255 - cycle, 255 - cycle));
					color[y * width + x] = Color.rgb(255 - cycle, 255 - cycle, 255 - cycle);
				}
			}

		mIQBitMap.copyPixelsFromBuffer(IntBuffer.wrap(color));
		cycle += 16;


	}


 }


