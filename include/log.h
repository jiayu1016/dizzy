#ifndef LOG_H
#define LOG_H

#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "DZY"

#define DO_LOG(LEVEL, FMT, ...)                 \
do {                                            \
    int ptid = (int)pthread_self();             \
    __android_log_print(LEVEL,                  \
        LOG_TAG,                                \
        "[%d][%s:%d]" FMT, ptid,                \
        __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while(0)

#define ALOGV(FMT, ...) DO_LOG(ANDROID_LOG_VERBOSE, FMT, ##__VA_ARGS__);
#define ALOGD(FMT, ...) DO_LOG(ANDROID_LOG_DEBUG,   FMT, ##__VA_ARGS__);
#define ALOGI(FMT, ...) DO_LOG(ANDROID_LOG_INFO,    FMT, ##__VA_ARGS__);
#define ALOGW(FMT, ...) DO_LOG(ANDROID_LOG_WARN,    FMT, ##__VA_ARGS__);
#define ALOGE(FMT, ...) DO_LOG(ANDROID_LOG_ERROR,   FMT, ##__VA_ARGS__);

#define PRINT(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__));

#endif
