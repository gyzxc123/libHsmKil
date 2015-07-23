
package com.honeywell.barcode1;

import java.io.IOException;

import com.hsm.barcode.*;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;

public class Barcode1Activity extends Activity {
    private static final String TAG = "Barcode1";
    private TextView m_BarcodeContentsView;
    private Decoder m_decDecoder = null;
    private DecodeResult m_decResult = null;
    private static final int m_DecodeTimeout = 4000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        m_BarcodeContentsView = (TextView) findViewById(R.id.textBarcodeContents);
        m_decResult = new DecodeResult();
        initSound();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public void onResume() {
        super.onResume(); // Always call the superclass method first
        Log.d(TAG, "onResume");
        m_decDecoder = new Decoder();
        m_decDecoder.connectToDecoder();
    }

    @Override
    public void onPause() {
        super.onPause(); // Always call the superclass method first

        Log.d(TAG, "onPause");
        m_decDecoder.disconnectFromDecoder();
        m_decDecoder = null;
    }

    public MediaPlayer mediaPlayer;
    private OnCompletionListener beepListener = new BeepListener();
    private static final float BEEP_VOLUME = 0.10f;

    /**
     * Creates the beep MediaPlayer in advance so that the sound can be
     * triggered with the least latency possible.
     */
    private void initSound() {
        if (mediaPlayer == null) {
            // The volume on STREAM_SYSTEM is not adjustable, and users found it
            // too loud,
            // so we now play on the music stream.
            setVolumeControlStream(AudioManager.STREAM_MUSIC);
            mediaPlayer = new MediaPlayer();
            mediaPlayer.setAudioStreamType(AudioManager.STREAM_RING);
            mediaPlayer.setOnCompletionListener(beepListener);
            AssetFileDescriptor file = getResources().openRawResourceFd(
                    R.raw.beep);
            try {
                mediaPlayer.setDataSource(file.getFileDescriptor(),
                        file.getStartOffset(), file.getLength());
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

    /* When the beep has finished playing, rewind to queue up another one. */
    private static class BeepListener implements OnCompletionListener {
        public void onCompletion(MediaPlayer mediaPlayer) {
            mediaPlayer.seekTo(0);
        }
    }

    /** Called when the user clicks the Scan button */
    public void onClickScan(View view) {
        // Do something in response to button
        Log.d(TAG, "onClickScan");
        // puts("scanning");
        m_decDecoder.enableSymbology(DecoderConfigValues.SymbologyID.SYM_QR);
        m_decDecoder.waitForDecodeTwo(m_DecodeTimeout, m_decResult);
        if (m_decResult.length > 0) {
            Log.d(TAG, "decode success");
            playSound();
            puts(m_decResult.barcodeData);
        } else {
            puts("no read");
        }
    }

    /** Called when the user clicks the Exit button */
    public void onClickExit(View view) {
        Log.d(TAG, "onClickExit");
        m_decDecoder.disconnectFromDecoder();
        m_decDecoder = null;
        m_decResult = null;
        finish();
        System.exit(0);
    }

    private void puts(String text) {
        m_BarcodeContentsView.setText(m_BarcodeContentsView.getText() + "\n"
                + text);
    }
}
