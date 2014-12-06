#include <stdlib.h>
#include "log.h"

using namespace std;

namespace dzy {

int Log::mFlag = 0;

Log::FTrace::FTrace(const char *function, int line, const char* message)
    : mMessage(message), mFunction(function), mLine(line) {
    if (debugSwitchOn() && flagEnabled(F_TRACE)) {
        string func(string("--> ") + mFunction);
        int ptid = (int)pthread_self();
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,
            "[%d][%s:%d] %s",
            ptid, func.c_str(), mLine, mMessage);
    }
}

Log::FTrace::~FTrace() {
    if (Log::debugSwitchOn() && Log::flagEnabled(F_TRACE)) {
        string func(string("<-- ") + mFunction);
        int ptid = (int)pthread_self();
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,
            "[%d][%s:%d] %s",
            ptid, func.c_str(), mLine, mMessage);
    }
}

void Log::setFlag(Flag f) {
    mFlag |= f;
}

void Log::clearFlag(Flag f) {
    mFlag &= ~f;
}

bool Log::flagEnabled(Flag f) {
    return mFlag & f;
}

void Log::setDebugSwitch(bool on) {
    if (on) mFlag |= F_SWITCH;
    else    mFlag &= ~F_SWITCH;
}

bool Log::debugSwitchOn() {
    return mFlag & F_SWITCH;
}


}
