package com.vam.vpusher.media;

import android.app.Activity;
import android.hardware.Camera;
import android.view.SurfaceHolder;

import com.vam.vpusher.LivePusher;
import com.vam.vpusher.MainActivityKt;

public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {

    private SurfaceHolder surfaceHolder;
    private CameraHelper cameraHelper;
    private int mBitrate;
    private int mFps;
    private boolean isLiving;

    private LivePusher livePusher;

    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        this.livePusher = livePusher;
        mBitrate = bitrate;
        mFps = fps;
        cameraHelper = new CameraHelper(activity, cameraId, width, height);
        cameraHelper.setOnChangedSizeListener(this);
        cameraHelper.setPreviewCallback(this);
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        this.surfaceHolder = surfaceHolder;
        cameraHelper.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (isLiving) {
            livePusher.native_pushVideo(data);
        }
    }

    @Override
    public void onChanged(int w, int h) {
        livePusher.setVideoEncInfo(w, h, mFps, mBitrate);
    }

    public void startLive() {
        isLiving = true;
    }

    public void stopLive() {
        isLiving = false;
    }

    public void release(){
        cameraHelper.release();
    }

}
