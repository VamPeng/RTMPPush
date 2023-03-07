package com.vam.vpusher;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.vam.vpusher.media.AudioChannel;
import com.vam.vpusher.media.VideoChannel;

public class LivePusher {

    private AudioChannel audioChannel;
    private VideoChannel videoChannel;

    static {
        System.loadLibrary("vpusher");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        videoChannel.switchCamera();
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }

    public void stopLive(){
        videoChannel.stopLive();
        audioChannel.stopLive();
        native_stop();
    }

    public void release(){
        native_release();
    }

    public native void native_init();

    public native void native_start(String path);

    public native void setVideoEncInfo(int width, int height, int fps, int bitrate);

    public native void native_pushVideo(byte[] data_);
    public native void native_stop();

    public native void native_release();
    public native void native_setAudioEncInfo(int sampleRates,int channels);

    public native int getInputSamples();

    public native void native_pushAudio(byte[] bytes);

}
