#ifndef LOG_H
#define LOG_H

#include <string>
#include <android/log.h>

namespace dzy {

class Log {
public:
    enum Flag {
        F_TRACE         = (1 << 0),
        F_GENERIC       = (1 << 1),
        F_MODEL         = (1 << 2),
        F_ANIMATION     = (1 << 3),
        F_BONE          = (1 << 4),
        F_GLES          = (1 << 5),
        F_KEY_EVENT     = (1 << 6),
        F_MOTION_EVENT  = (1 << 7),
        F_SENSOR_EVENT  = (1 << 8),
        F_APP_CMD_EVENT = (1 << 9),
        F_EVENT         = F_KEY_EVENT | F_MOTION_EVENT | F_SENSOR_EVENT | F_APP_CMD_EVENT,
        F_ALWAYS        = F_TRACE | F_GENERIC | F_MODEL | F_GLES | F_EVENT,
        F_NEVER         = 0,
        F_SWITCH        = (1 << 31),
    };
    class FTrace {
    public:
        FTrace(const char *function, int line, const char* message);
        ~FTrace();
    private:
        const char* mMessage;
        const char* mFunction;
        const int   mLine;
    };
    static void setFlag(Flag f);
    static void clearFlag(Flag f);
    static bool flagEnabled(Flag f);
    static void setDebugSwitch(bool on);
    static bool debugSwitchOn();
private:
    static int      mFlag;
    std::string     mName;
};

} //namespace

#ifndef LOG_TAG
#define LOG_TAG     "DZY"
#endif

#define DO_LOG(LEVEL, FMT, ...)                 \
do {                                            \
    int ptid = (int)pthread_self();             \
    __android_log_print(LEVEL,                  \
        LOG_TAG,                                \
        "[%d][%s:%d]" FMT, ptid,                \
        __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while(0)

// these macros are not affected by debug switch, controlled only by android log level
#define ALOGV(FMT, ...) DO_LOG(ANDROID_LOG_VERBOSE, FMT, ##__VA_ARGS__)
#define ALOGD(FMT, ...) DO_LOG(ANDROID_LOG_DEBUG,   FMT, ##__VA_ARGS__)
#define ALOGI(FMT, ...) DO_LOG(ANDROID_LOG_INFO,    FMT, ##__VA_ARGS__)
#define ALOGW(FMT, ...) DO_LOG(ANDROID_LOG_WARN,    FMT, ##__VA_ARGS__)
#define ALOGE(FMT, ...) DO_LOG(ANDROID_LOG_ERROR,   FMT, ##__VA_ARGS__)

#define DO_DEBUG(FLAG, FMT, ...)                        \
if (Log::debugSwitchOn() && Log::flagEnabled(FLAG)) {   \
    do {                                                \
        int ptid = (int)pthread_self();                 \
        __android_log_print(ANDROID_LOG_VERBOSE,        \
            LOG_TAG,                                    \
            "[%d][%s:%d]" FMT, ptid,                    \
            __FUNCTION__, __LINE__, ##__VA_ARGS__);     \
    } while(0);                                         \
}

// raw information, no pid, function, line
#define DO_DUMP(FLAG, FMT, ...)                         \
if (Log::debugSwitchOn() && Log::flagEnabled(FLAG)) {   \
    do {                                                \
        __android_log_print(ANDROID_LOG_INFO,           \
            LOG_TAG,                                    \
            FMT,                                        \
            ##__VA_ARGS__);                             \
    } while(0);                                         \
}

// these macros are affected by debug switch
#define DEBUG(FLAG, FMT, ...)   DO_DEBUG(FLAG, FMT, ##__VA_ARGS__)
#define DUMP(FLAG, FMT, ...)    DO_DUMP(FLAG, FMT, ##__VA_ARGS__)
#define TRACE(MSG)              Log::FTrace __logtrace__(__FUNCTION__, __LINE__, MSG)

#endif
