package com.vam.vpusher.media;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.vam.vpusher.LivePusher;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {

    private int inputSampleSizes;
    private LivePusher livePusher;
    private AudioRecord audioRecord;
    private int channels = 2;
    private boolean isLiving = false;
    private ExecutorService executor = Executors.newSingleThreadExecutor();

    public AudioChannel(LivePusher livePusher) {
        this.livePusher = livePusher;

        int channelConfig = channels == 2 ? AudioFormat.CHANNEL_IN_STEREO : AudioFormat.CHANNEL_IN_MONO;

        livePusher.native_setAudioEncInfo(44100,channels);
        inputSampleSizes = livePusher.getInputSamples() * 2;

        int minBufferSize = AudioRecord.getMinBufferSize(44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;
        minBufferSize = Math.max(minBufferSize, inputSampleSizes);
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                44100,
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
    }

    public void startLive() {
        isLiving = true;
        executor.submit(new AudioTask());
    }

    public void setChannels(int channels) {
        this.channels = channels;
    }

    public void stopLive() {
        isLiving = false;
    }

    public void release(){
        audioRecord.release();
    }

    class AudioTask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();
            byte[] bytes = new byte[inputSampleSizes];
            while (isLiving) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    livePusher.native_pushAudio(bytes);
                }
            }
        }
    }

}
