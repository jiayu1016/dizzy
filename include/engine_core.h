#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include <memory>
#include <chrono>
#include <android_native_app_glue.h>
#include "utils.h"

namespace dzy {

class EngineContext;
class Scene;
class Render;
class EngineCore 
    : public std::enable_shared_from_this<EngineCore>
    , private noncopyable {
public:
    EngineCore();
    virtual ~EngineCore();

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
    virtual bool create();
    /// release native activity
    virtual void destory();

    /// initialize view
    ///
    ///     the egl/gl context has been initialized,
    ///     do one time setup for something that need egl/gl context
    ///
    ///     @return true if success, false otherwise
    virtual bool start() = 0;

    /// release view
    virtual void stop() = 0;

    /// update the scene data
    ///
    ///     this is where app update scene data,
    ///     called once each frame
    ///
    ///     @param interval the time interval in millisecond between two consecutive call
    ///     @return true if success, false otherwise
    virtual bool update(long interval);

    /// get a scene to draw by render
    ///
    ///     this function is called every frame.
    ///     @return the current scene to be drawn
    virtual std::shared_ptr<Scene> getScene() = 0;

    std::string getIntentString(const std::string& name);
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
    bool updateFrame();

private:
    std::shared_ptr<EngineContext>                  mEngineContext;
    struct android_app*                             mApp;
    JNIEnv*                                         mJNIEnv;
    std::chrono::high_resolution_clock::time_point  mLastUpdated;
    bool                                            mFirstFrameUpdated;
};

} // namespace dzy

#endif
