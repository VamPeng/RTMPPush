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
    }

    public native void native_init();

    public native void native_start(String path);

    public native void setVideoEncInfo(int width, int height, int fps, int bitrate);

    public native void native_pushVideo(byte[] data_);

}
