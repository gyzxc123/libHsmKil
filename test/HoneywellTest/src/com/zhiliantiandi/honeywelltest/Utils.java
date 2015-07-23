
package com.zhiliantiandi.honeywelltest;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import com.zhiliantiandi.honeywelltest.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;

public class Utils {
    public static final int SOUND_TYPE_SUCCESS = 0;
    public static final int SOUND_TYPE_BEEP = 1;
    private static final String TAG = "BarCodeTestUtils";

    private static Utils mInstance;
    private Context mContext;
    private SoundPool mSoundPool;

    private int mSoundBeepId;
    private int mSoundSuccessId;
    private int rawPicCount = 0;
    private int pngPicCount = 0;

    public static Utils getInstance() {
        if (mInstance == null) {
            mInstance = new Utils();
        }
        return mInstance;
    }

    public void init(Context context) {
        mContext = context;
        loadSoundResources();
    }

    public void release() {
        if (mSoundPool != null) {
            mSoundPool.release();
            mSoundPool = null;
        }
    }

    public void playSound(int soundType) {
        int soundResId = mSoundSuccessId;
        switch (soundType) {
            case SOUND_TYPE_SUCCESS:
                soundResId = mSoundSuccessId;
                break;
            case SOUND_TYPE_BEEP:
                soundResId = mSoundBeepId;
                break;
            default:
                break;
        }
        mSoundPool.play(soundResId, getVolume(), getVolume(), 1, 0, 1.0f);
    }

    public boolean StoreByteImage(byte[] imageData, int quality, String expName) {
        File sdImageMainDirectory = new File("/sdcard/Test");
        File imageFilePath = new File(sdImageMainDirectory.getPath() + "/Images/");

        imageFilePath.mkdirs();
        FileOutputStream fileOutputStream1 = null;
        BufferedOutputStream bos = null;
        String nameFile = "";
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = 1;

            int width = 0;
            int height = 0;
            Bitmap myImage = null;
            if (expName == "raw") {
                width = 832;
                height = 640;
                // Deal with the Delivery task if it is running
                nameFile = imageFilePath.toString() + "/" + "RawImage_" + rawPicCount + ".raw";
                fileOutputStream1 = new FileOutputStream(nameFile);
                bos = new BufferedOutputStream(fileOutputStream1);
                bos.write(imageData);
                Log.d(TAG, "RAW file save success");
                rawPicCount++;
            }
            else if (expName == "png") {
                width = 832;
                height = 640;
                myImage = renderCroppedGreyscaleBitmap(imageData, 0, 0, width, height, width);
                Log.d(TAG, "imageData length " + imageData.length);
                Log.d(TAG, "Preview width: " + width +
                        " Preview height: " + height);
                nameFile = "Image_";

                if (myImage.isMutable())
                    Log.d(TAG, "Image is mutable");

                String myNameFile = imageFilePath.toString() + "/" + nameFile + pngPicCount
                        + ".png";
                fileOutputStream1 = new FileOutputStream(myNameFile);
                bos = new BufferedOutputStream(fileOutputStream1);
                myImage.compress(Bitmap.CompressFormat.PNG, quality, bos);
                Log.d(TAG, "PNG file save success");
                pngPicCount++;
            }
            bos.flush();
            bos.close();

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            Log.d(TAG, "Image save was unsuccessful");
            e.printStackTrace();
        }
        return true;
    }

    public Bitmap renderCroppedGreyscaleBitmap(byte[] data, int top, int left, int width,
            int height, int dataWidth) {

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

    private void loadSoundResources() {
        if (mSoundPool == null) {
            mSoundPool = new SoundPool(3, AudioManager.STREAM_RING, 0);
        }
        mSoundBeepId = mSoundPool.load(mContext, R.raw.beep, 1);
        mSoundSuccessId = mSoundPool.load(mContext, R.raw.decoded, 1);
    }

    private float getVolume() {
        AudioManager am = (AudioManager) mContext
                .getSystemService(Context.AUDIO_SERVICE);
        return am.getStreamVolume(AudioManager.STREAM_RING) / 10f;
    }
}
