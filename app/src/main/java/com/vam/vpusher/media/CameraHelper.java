package com.vam.vpusher.media;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.CameraInfo;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.vam.vpusher.MainActivityKt;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

public class CameraHelper implements SurfaceHolder.Callback, Camera.PreviewCallback {

    private Activity activity;
    private int height;
    private int width;
    private int cameraId;
    private Camera camera;
    private byte[] buffer;
    private SurfaceHolder surfaceHolder;
    private Camera.PreviewCallback previewCallback;
    private int rotation;
    private OnChangedSizeListener sizeListener;
    byte[] bytes;

    public CameraHelper(Activity activity, int cameraId, int width, int height) {
        this.activity = activity;
        this.cameraId = cameraId;
        this.width = width;
        this.height = height;
    }

    public void setOnChangedSizeListener(OnChangedSizeListener listener) {
        sizeListener = listener;
    }

    private int getBack() {
        return Camera.CameraInfo.CAMERA_FACING_BACK;
    }

    private int getFront() {
        return Camera.CameraInfo.CAMERA_FACING_FRONT;
    }

    public void switchCamera() {
        if (cameraId == getBack()) {
            cameraId = getFront();
        } else {
            cameraId = getBack();
        }
        stopPreview();
        startPreview();
    }

    private void stopPreview() {
        if (camera != null) {
            camera.setPreviewCallback(null);
            camera.stopPreview();
            camera.release();
            camera = null;
        }
    }

    private void startPreview() {
        // 获得camera
        camera = Camera.open(cameraId);
        // 配置属性
        Camera.Parameters parameters = camera.getParameters();
        // 设置数据格式
        parameters.setPreviewFormat(ImageFormat.NV21);

        // 设置宽高
        setPreviewSize(parameters);
        // 设置摄像头、图像传感器的角度、方向
        setPreviewOrientation(parameters);
        camera.setParameters(parameters);

        buffer = new byte[width * height * 3 / 2];
        bytes = new byte[buffer.length];

        // 数据缓存区
        camera.addCallbackBuffer(buffer);
        camera.setPreviewCallbackWithBuffer(this);
        try {
            camera.setPreviewDisplay(surfaceHolder);
            camera.startPreview();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void setPreviewSize(Camera.Parameters parameters) {
        // 获取摄像头支持的宽、高
        List<Camera.Size> supportedPreviewSizes = parameters.getSupportedPreviewSizes();
        Camera.Size size = supportedPreviewSizes.get(0);
        tag("支持 " + size.width + "x" + size.height);
        // 选择一个与设置的差距最小的支持分辨率
        // 10x10 20x20 30x30
        int m = Math.abs(size.height * size.width - width * height);
        supportedPreviewSizes.remove(0);
        Iterator<Camera.Size> iterator = supportedPreviewSizes.iterator();
        while (iterator.hasNext()) {
            Camera.Size next = iterator.next();
            tag("支持 " + size.width + "x" + next.height);
            int n = Math.abs(next.height * next.width - width * height);
            if (n < m) {
                m = n;
                size = next;
            }
        }
        width = size.width;
        height = size.height;
        parameters.setPreviewSize(width, height);
        tag("设置分辨率: " + size.width + "-" + size.height);

    }

    private void setPreviewOrientation(Camera.Parameters parameters) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                sizeListener.onChanged(height, width);
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                sizeListener.onChanged(height, width);
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                sizeListener.onChanged(height, width);
                break;
        }
        int result;
        if (info.facing == getFront()) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;
        } else {
            result = (info.orientation - degrees + 360) % 360;
        }
        camera.setDisplayOrientation(result);
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        this.surfaceHolder = surfaceHolder;
        surfaceHolder.addCallback(this);
    }

    public void setPreviewCallback(Camera.PreviewCallback previewCallback) {
        this.previewCallback = previewCallback;
    }

    private void tag(String content) {

        MainActivityKt.logI(this, content);

    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        switch (rotation) {
            case Surface.ROTATION_0:
                rotation90(data);
                break;
            case Surface.ROTATION_90:
                break;
            case Surface.ROTATION_270:
                break;
        }
        previewCallback.onPreviewFrame(bytes, camera);
        camera.addCallbackBuffer(buffer);
    }

    private void rotation90(byte[] data) {
        int index = 0;
        int ySize = width * height;


        int uvHeight = height / 2;
        if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK) {

            //将y的数据旋转之后 放入新的byte数组
            for (int i = 0; i < width; i++) {
                for (int j = height - 1; j >= 0; j--) {
                    bytes[index++] = data[width * j + i];
                }
            }

            for (int i = 0; i < width; i += 2) {
                for (int j = uvHeight - 1; j >= 0; j--) {
                    //u
                    bytes[index++] = data[ySize + width * j + i];

                    //v
                    bytes[index++] = data[ySize + width * j + i + 1];
                }
            }
        } else {
            for (int i = 0; i < width; i++) {
                int nPos = width - 1;
                for (int j = 0; j < height; j++) {
                    bytes[index++] = data[nPos - i];
                    nPos += width;
                }
            }

            for (int i = 0; i < width; i += 2) {
                int nPos = ySize + width - 1;
                for (int j = 0; j < uvHeight; j++) {
                    bytes[index++] = data[nPos - i - 1];
                    bytes[index++] = data[nPos - i];
                    nPos += width;
                }
            }
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        stopPreview();
        startPreview();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPreview();
    }

    public void release() {
        surfaceHolder.removeCallback(this);
        stopPreview();
    }

    interface OnChangedSizeListener {
        void onChanged(int w, int h);
    }

}
