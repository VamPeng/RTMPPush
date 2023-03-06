#include <jni.h>
#include <string>
#include <pthread.h>

#include "x264.h"
#include "VideoChannel.h"
#include "SafeQueue.h"
#include "AudioChannel.h"
#include "librtmp/rtmp.h"
#include "macro.h"

VideoChannel *videoChannel;

int isStart = 0;
pthread_t pid;

uint32_t start_time;
int readyPushing = 0;

SafeQueue<RTMPPacket *> packets;

void callback(RTMPPacket *packet) {
    if (packet) {
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        packets.put(packet);
    }
}

void releasePacket(RTMPPacket *&packet) {
    RTMPPacket_Free(packet);
    delete packet;
    packet = 0;
}

void *start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("alloc rtmp失败");
        return NULL;
    }
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("设置url失败:%s", url);
        return NULL;
    } else{
        LOGE("设置url成功:%s",url);
    }
    rtmp->Link.timeout = 5;
    RTMP_EnableWrite(rtmp);
    RTMP_Connect(rtmp, 0);

    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接失败");
        return NULL;
    } else{
        LOGE("连接成功");
    }

    start_time = RTMP_GetTime();
    readyPushing = 1;
    packets.setWork(1);
    RTMPPacket *packet = 0;
    while (readyPushing) {
        // 队列取数据 packets
        packets.get(packet);
        LOGE("取出一帧数据");
        if (!readyPushing) {
            LOGE("readyPushing:false");
            break;
        }
        if (!packet) {
            LOGE("packet NULL");
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;
        LOGE("发送数据");
        ret = RTMP_SendPacket(rtmp, packet, 1);
        LOGE("发送结果: %d",ret);
        // 释放packet
        releasePacket(packet);

    }

    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);

    return 0;

}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_vam_vpusher_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vpusher_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vpusher_LivePusher_setVideoEncInfo(JNIEnv *env, jobject thiz, jint width, jint height,
                                                jint fps, jint bitrate) {

    if (!videoChannel) {
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vpusher_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (isStart) {
        return;
    }
    isStart = 1;
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);

    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vpusher_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data_) {

    if (!videoChannel || !readyPushing) {
        return;
    }

    jbyte *data = env->GetByteArrayElements(data_, NULL);

    // data NV21
    videoChannel->encodeData(data);

    env->ReleaseByteArrayElements(data_, data, 0);
}