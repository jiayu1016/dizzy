#ifndef UTILS_H
#define UTILS_H

#include "log.h"
#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace dzy {

class noncopyable {
public:
    noncopyable() {};
    ~noncopyable() {};
private:
    noncopyable(noncopyable const &);
    noncopyable & operator=(noncopyable const &);
};

template <typename T>
class Singleton : private noncopyable {
public:
    static T *get() {
        if (!mInstance) mInstance = new T;
        return mInstance;
    }

    static void release() {
        if (mInstance) {
            delete mInstance;
            mInstance = nullptr;
        }
    }
protected:
    Singleton() {};
    virtual ~Singleton() {};
    static T *mInstance;
};

template <typename T>
T *Singleton<T>::mInstance = NULL;

class MeasureDuration {
public:
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::milliseconds MilliSeconds;
    typedef std::chrono::microseconds MicroSeconds;

    MeasureDuration()
        : mStart(Clock::now()) { }

    long long getMilliSeconds() {
        Clock::time_point end = Clock::now();
        MilliSeconds ms = std::chrono::duration_cast<MilliSeconds>(
            end - mStart);
        return ms.count();
    }

    long long getMicroSeconds() {
        Clock::time_point end = Clock::now();
        MicroSeconds ms = std::chrono::duration_cast<MicroSeconds>(
            end - mStart);
        return ms.count();
    }

    static long getInterval(const Clock::time_point& timePoint) {
        Clock::time_point end = Clock::now();
        MilliSeconds ms = std::chrono::duration_cast<MilliSeconds>(
            end - timePoint);
        return ms.count();
    }

private:
    Clock::time_point mStart;
};

class Utils {
public:
    static void dump(Log::Flag f, const char *msg, const glm::mat4& mat4) {
        if (!Log::debugSwitchOn() || !Log::flagEnabled(f))
            return;
        float const * buf = glm::value_ptr(mat4);
        DUMP(f, "********** %s ************", msg);
        for (int i=0; i<16; i+=4) {
            DUMP(f, "%+08.6f %+08.6f %+08.6f %+08.6f",
                buf[i], buf[i+1], buf[i+2], buf[i+3]);
        }
    }

    static void dump(Log::Flag f, const char *msg, const glm::vec3& vec3) {
        DUMP(f, "%s: %+08.6f %+08.6f %+08.6f",
            msg, vec3.x, vec3.y, vec3.z);
    }

    static void dump(Log::Flag f, const char *msg, const glm::vec4& vec4) {
        DUMP(f, "%s: %+08.6f %+08.6f %+08.6f %+08.6f",
            msg, vec4.x, vec4.y, vec4.z, vec4.w);
    }

};

}

#endif
