//
// Created by Yuhui Peng on 2023/3/6.
//

#ifndef VPUSHER_AUDIOCHANNEL_H
#define VPUSHER_AUDIOCHANNEL_H


#include <cstdio>
#include "faac.h"
#include "librtmp/rtmp.h"

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket *packet);
public:

    AudioChannel();
    ~AudioChannel();

    void setAudioEncInfo(int samplesInHZ, int channels);
    void setAudioCallback(AudioCallback audioCallback);

    void encodeData(int8_t *data);
    int getInputSamples();

    RTMPPacket* getAudioTag();
private :
    AudioCallback  audioCallback;
    int channels;
    faacEncHandle  audioCodec = 0;
    u_long inputSamples;
    u_long maxOutputBytes;
    u_char *buffer = 0;
};


#endif //VPUSHER_AUDIOCHANNEL_H
