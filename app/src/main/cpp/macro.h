//
// Created by Yuhui Peng on 2023/3/6.
//

#ifndef VPUSHER_MACRO_H
#define VPUSHER_MACRO_H

#include "android/log.h"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"VamPush",__VA_ARGS__)

#define DELETE(obj) if(obj){delete obj; obj=0;}


#endif //VPUSHER_MACRO_H
