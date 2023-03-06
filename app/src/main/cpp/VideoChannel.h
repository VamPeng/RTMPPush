//
// Created by Yuhui Peng on 2023/3/6.
//

#ifndef VPUSHER_VIDEOCHANNEL_H
#define VPUSHER_VIDEOCHANNEL_H


#include <cstdint>
#include "x264.h"
#include "librtmp/rtmp.h"

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket *packet);

public:

    void setVideoEncInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);

    void setVideoCallback(VideoCallback videoCallback);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;
    int uvSize;
    x264_t *videoCodec;
    // 一帧图片
    x264_picture_t *pic_in;

    void sendSpsPps(uint8_t sps[100], uint8_t pps[100], int len, int len1);

    void sendFrame(int i, uint8_t *string, int i1);

    VideoCallback videoCallback;
};


#endif //VPUSHER_VIDEOCHANNEL_H
