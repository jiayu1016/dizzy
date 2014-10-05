#ifndef UTILS_H
#define UTILS_H

namespace dzy {

template <typename T>
struct ArrayDeleter {
    void operator ()(T *p) { delete[] p; }
};

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

}

#endif
