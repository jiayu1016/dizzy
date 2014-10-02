#ifndef NATIVE_APP_H
#define NATIVE_APP_H

#include <memory>

namespace dzy {

class NativeApp {
public:
    virtual bool initApp() { return true; };
    virtual bool releaseApp() { return true; };
    virtual bool initView() { return true; };
    virtual bool releaseView() { return true; };

    virtual bool drawScene() { return true; };
};

extern std::shared_ptr<NativeApp> nativeInit();

} // namespace dzy

#endif
