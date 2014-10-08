#ifndef UTILS_H
#define UTILS_H

#include <chrono>

namespace dzy {

template <typename T>
class Singleton {
public:
    static T *get() {
        if (!mInstance) mInstance = new T;
        return mInstance;
    }
protected:
    Singleton() {};
    static T *mInstance;
};

class noncopyable {
public:
    noncopyable() {};
    ~noncopyable() {};
private:
    noncopyable(noncopyable const &);
    noncopyable & operator=(noncopyable const &);
};

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

private:
    Clock::time_point mStart;
};

}

#endif
