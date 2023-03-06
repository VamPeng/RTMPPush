//
// Created by Yuhui Peng on 2023/3/6.
//

#include <cstring>
#include "VideoChannel.h"
#include "x264.h"
#include "librtmp/rtmp.h"
#include "macro.h"

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;
    ySize = width * height;
    uvSize = ySize / 4;

    // 初始化x264的编码器
    // 编码：压缩图像处理数据
    x264_param_t param;
    x264_param_default_preset(&param, x264_preset_names[0], x264_tune_names[7]);

    // base_line 3.2 编码复杂度
    param.i_level_idc = 32;

    // 输入数据格式 NV21 服务器 i420
    param.i_csp = X264_CSP_I420;

    param.i_width = width;
    param.i_height = height;
    // 无b帧
    param.i_bframe = 0;

    // 参数表示码率控制，CQP(恒定质量),CRF(恒定码率),ABR(平均码率)
    param.rc.i_rc_method = X264_RC_ABR;
    // 码率
    param.rc.i_bitrate = bitrate / 1000;
    // 瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    // 设置了i_vbv_max_bitrate必须设置此参数，码率控制区大小，单位kbps
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    // 音视频特点
    // 算出一帧的时间间隔
    // 帧率分子 帧数
    param.i_fps_num = fps;
    // 帧率分母 s
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    // 用fps而不是时间戳来计算帧间距离
    param.b_vfr_input = 0;
    // 帧距离(关键帧) 2s一个关键帧
    param.i_keyint_max = fps * 2;
    // 是否复制sps和pps放在每个关键帧的前面，该参数设置是让每个关键帧都附带sps/pps
    // sps:苏烈参数集
    // pps:图像参数集
    param.b_repeat_headers = 1;
    // 0则默认开启多线程编码，1是单线程
    param.i_threads = 1;

    x264_param_apply_profile(&param, x264_profile_names[0]);

    videoCodec = x264_encoder_open(&param);
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);

}


void VideoChannel::setVideoCallback(VideoCallback videoCallback) {
    this->videoCallback = videoCallback;
}

void VideoChannel::encodeData(int8_t *data) {
    // 数据 data
    memcpy(pic_in->img.plane[0], data, ySize);

    for (int i = 0; i < uvSize; ++i) {
        // v数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1); //u 1 3 5 7 9
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2); //v 0 2 4 6 8
    }

    // NALU单元
    x264_nal_t *pp_nal;
    //编码出有几个数据 (多少个NALU单元)
    int pi_nal;
    x264_picture_t pic_out;
    x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);
    int sps_len;
    int pps_len;
    uint8_t sps[100];
    uint8_t pps[100];
    for (int i = 0; i < pi_nal; ++i) {
        if (pp_nal[i].i_type == NAL_SPS) {
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        } else if (pp_nal[i].i_type == NAL_PPS) {
            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);

            sendSpsPps(sps, pps, sps_len, pps_len);
        } else {
            // 非关键帧
            sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }

    }

}

// 格式 RTMP
void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    LOGE("SPS&PPS");
    int bodySizy = 13 + sps_len + 3 + pps_len;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySizy);

    int i = 0;
    // 固定头
    packet->m_body[i++] = 0x17;

    // 类型
    packet->m_body[i++] = 0x00;

    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    // 版本
    packet->m_body[i++] = 0x01;
    // 编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;

    // 整个sps
    packet->m_body[i++] = 0xE1;
    // sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySizy;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;

    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    videoCallback(packet);

}

void VideoChannel::sendFrame(int type, uint8_t *payload, int i_payload) {
    if (payload[2] == 0x00) {
        i_payload -= 4;
        payload += 4;
    } else {
        i_payload -= 3;
        payload += 3;
    }

    int bodySize = 9 + i_payload;
    RTMPPacket *packet = new RTMPPacket;

    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {
        packet->m_body[0] = 0x17;
        LOGE("关键帧");
    }
    packet->m_body[1] = 0x01;
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;

    packet->m_body[5] = (i_payload >> 24) & 0xff;
    packet->m_body[6] = (i_payload >> 16) & 0xff;
    packet->m_body[7] = (i_payload >> 8) & 0xff;
    packet->m_body[8] = (i_payload) & 0xff;

    memcpy(&packet->m_body[9], payload, i_payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    videoCallback(packet);

}
