#ifndef EGLRENDERING_DEBUG_H
#define EGLRENDERING_DEBUG_H

#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


#endif //EGLRENDERING_DEBUG_H
