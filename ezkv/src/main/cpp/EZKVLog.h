//
// Created by chidh on 2021/3/24.
//

#ifndef EZKV_EZKVLOG_H
#define EZKV_EZKVLOG_H

#include <android/log.h>

#define LOG_TAG "EZKVTAG"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif //EZKV_EZKVLOG_H
