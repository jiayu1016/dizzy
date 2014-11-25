#ifndef NATIVE_CORE_H
#define NATIVE_CORE_H

#include <memory>
#include <android_native_app_glue.h>
#include "utils.h"

namespace dzy {

class EngineContext;
class Scene;
class Render;
class NativeCore 
    : public std::enable_shared_from_this<NativeCore>
    , private noncopyable {
public:
    explicit NativeCore();
    virtual ~NativeCore();

    bool init(struct android_app* app);
    void fini();
    void mainLoop();

    // event processing
    virtual void appCmd(int32_t cmd);
    virtual int32_t inputKeyEvent(int action, int code);
    virtual int32_t inputMotionEvent(int action);

    /// initialize nativie activity
    ///
    ///     the egl/gl context is not initialized yet,
    ///     do one time setup for something doesn't need egl/gl context.
    ///
    ///     @return true if success, false otherwise
    virtual bool initActivity();
    /// release native activity
    virtual bool releaseActivity();

    /// initialize view
    ///
    ///     the egl/gl context has been initialized,
    ///     do one time setup for something that need egl/gl context
    ///
    ///     @return true if success, false otherwise
    virtual bool initView() = 0;

    /// release view
    virtual bool releaseView() = 0;

    /// get a scene to draw by render
    ///
    ///     this function is called every frame.
    ///     @return the current scene to be drawn
    virtual std::shared_ptr<Scene> getScene() = 0;

    std::shared_ptr<EngineContext> getEngineContext();

    friend class EngineContext;
private:
    /*
     * return 0, the framework will continue to handle the event
     * return 1, the framework will stop to handle the event
     */
    static void handleAppCmd(struct android_app* app, int32_t cmd);
    static int32_t handleInputEvent(struct android_app* app, AInputEvent* event);
    int32_t inputEvent(AInputEvent* event);

    std::shared_ptr<EngineContext> mEngineContext;

    struct android_app * mApp;
};

} // namespace dzy

#endif
