//
// Created by Yuhui Peng on 2023/3/6.
//

#include <cstring>
#include "AudioChannel.h"
#include "macro.h"

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {
    if (buffer) {
        delete buffer;
    }

    //释放编码器
    if (audioCodec) {
        faacEncClose(audioCodec);
        audioCodec = 0;
    }
}

void AudioChannel::encodeData(int8_t *data) {
    int bytelen = faacEncEncode(audioCodec,
                                reinterpret_cast<int32_t *>(data),
                                inputSamples, buffer,
                                maxOutputBytes);
    if (bytelen > 0) {
        int bodySize = 2 + bytelen;
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, bodySize);
        packet->m_body[0] = 0xAF;
        if (channels == 1) {
            packet->m_body[0] = 0xAE;
        }
        packet->m_body[1] = 0x01;

        memcpy(&packet->m_body[2], buffer, bytelen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        LOGI("AudioChannel 发送一帧音频数据 packet.size =  d%", packet->m_nBodySize);
        audioCallback(packet);
    }
}

void AudioChannel::setAudioEncInfo(int samplesInHZ, int channels) {
    this->channels = channels;
    audioCodec = faacEncOpen(samplesInHZ, channels, &inputSamples, &maxOutputBytes);

    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
    config->mpegVersion = MPEG4;
    config->aacObjectType = LOW;
    config->inputFormat = FAAC_INPUT_16BIT;
    config->outputFormat = 0;

    faacEncSetConfiguration(audioCodec, config);

    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}


RTMPPacket *AudioChannel::getAudioTag() {
    u_char *buf;
    u_long len;
    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);
    int bodySize = 2 + len;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0xAF;
    if (channels == 1) {
        packet->m_body[0] = 0xAE;
    }
    packet->m_body[1] = 0x00;
    memcpy(&packet->m_body[2], buf, len);
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {
    this->audioCallback = audioCallback;
}



