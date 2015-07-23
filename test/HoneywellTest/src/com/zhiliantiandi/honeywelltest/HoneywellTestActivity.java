
package com.zhiliantiandi.honeywelltest;

import com.hsm.barcode.*;
import com.zhiliantiandi.honeywelltest.R;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.TextView;

public class HoneywellTestActivity extends Activity {
    private TextView mTextView;
    private Decoder mDecoder = null;
    private DecodeResult mDecodeResult = null;
    private static final int mDecodeTimeout = 4000;

    private boolean mContinue;
    private long mIntervalTime;
    private long mElapsedTime;

    private Utils mUtils;
    private DecodeThread mDecodeThread;
    public Handler mDecodeHandler;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            mTextView.append("Decoder elapsed time is " + mElapsedTime + " MS.\nResult:\n");
            if (mDecodeResult.length > 0) {
                mTextView.append(mDecodeResult.barcodeData);
            } else {
                mTextView.append("no reading or timeout.");
            }
            mTextView.append("\n\n");
            mUtils.playSound(Utils.SOUND_TYPE_BEEP);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bar_code_test);
        mTextView = (TextView) findViewById(R.id.text_view);
        mTextView.setMovementMethod(ScrollingMovementMethod.getInstance());
        mDecodeResult = new DecodeResult();
        mUtils = Utils.getInstance();
        mUtils.init(this);
        mIntervalTime = 100;
        mContinue = false;
    }

    public void onResume() {
        super.onResume();
        mDecoder = new Decoder();
        mDecoder.connectToDecoder();
        mDecoder.enableSymbology(DecoderConfigValues.SymbologyID.SYM_QR);
        mDecodeThread = new DecodeThread();
        mDecodeThread.start();
    }

    public void onPause() {
        super.onPause();
        mDecodeHandler.removeMessages(0);
        if (mDecoder != null) {
            mDecoder.disconnectFromDecoder();
            mDecoder = null;
        }
    }

    public void onDestroy() {
        super.onDestroy();
        mUtils.release();
    }

    public void singleShootOnClick(View view) {
        mTextView.append("single shooting...\n");
        mDecodeHandler.sendEmptyMessage(0);
    }

    public void continuousShootOnClick(View view) {
        if (mContinue) {
            mContinue=false;
            mDecodeHandler.removeMessages(0);
        } else {
            mTextView.append("continuous shooting...\n");
            mContinue = true;
            new Thread(new ContinuousShootRunnable()).start();
        }
    }

    public void exitOnClick(View view) {
        mTextView.append("exit");
        finish();
    }

    public void clearOnClick(View view) {
        mTextView.setText("");
    }

    public void selectTimeOnClick(View view) {

    }

    private class ContinuousShootRunnable implements Runnable {
        @Override
        public void run() {
            while (mContinue) {
                mDecodeHandler.sendEmptyMessage(0);
                try {
                    Thread.sleep(mIntervalTime);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class DecodeThread extends Thread {
        private long mStartTime;
        private long mEndTime;

        public void run() {
            Looper.prepare();
            mDecodeHandler = new Handler() {
                public void handleMessage(Message msg) {
                    mStartTime = System.currentTimeMillis();
                    mDecoder.waitForDecodeTwo(mDecodeTimeout, mDecodeResult);
                    mEndTime = System.currentTimeMillis();
                    mElapsedTime = mEndTime - mStartTime;
                    mHandler.sendEmptyMessage(0);
                }
            };
            Looper.loop();
        }
    }
}
