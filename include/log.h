#ifndef LOG_H
#define LOG_H

#include <sys/types.h>
#include <unistd.h>
#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "DZY"

#define DO_LOG(LEVEL, FMT, ...)                 \
do {                                            \
    int pid = getpid();                         \
    int ptid = (int)pthread_self();             \
    __android_log_print(LEVEL,                  \
        LOG_TAG,                                \
        "[%d:%d][%s:%d]" FMT, pid, ptid,        \
        __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while(0)

#define ALOGD(FMT, ...) DO_LOG(ANDROID_LOG_DEBUG, FMT, ##__VA_ARGS__);
#define ALOGI(FMT, ...) DO_LOG(ANDROID_LOG_INFO,  FMT, ##__VA_ARGS__);
#define ALOGW(FMT, ...) DO_LOG(ANDROID_LOG_WARN,  FMT, ##__VA_ARGS__);
#define ALOGE(FMT, ...) DO_LOG(ANDROID_LOG_ERROR, FMT, ##__VA_ARGS__);

#endif
