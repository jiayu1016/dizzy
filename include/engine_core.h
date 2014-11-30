#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include <memory>
#include <chrono>
#include <android_native_app_glue.h>
#include <gestureDetector.h>
#include "utils.h"

namespace dzy {

class EngineContext;
class Scene;
class Render;
class EngineCore 
    : public std::enable_shared_from_this<EngineCore>
    , private noncopyable {
public:
    enum GestureState {
        GESTURE_DRAG_START,
        GESTURE_DRAG_MOVE,
        GESTURE_DRAG_END,
        GESTURE_PINCH_START,
        GESTURE_PINCH_MOVE,
    };

    EngineCore();
    virtual ~EngineCore();

    bool init(struct android_app* app);
    void fini();
    void mainLoop();

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
    virtual bool start();

    /// release view
    virtual void stop();

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

    /// callback to handle motion event
    virtual bool handleMotion(int action);

    /// callback to handle key input event
    ///
    ///     @return true stop furthur processing the event, false otherwise
    virtual bool handleKey(int code);

    /// callback to handle double tap gesture
    ///
    ///     @return true stop furthur processing the event, false otherwise
    virtual bool handleDoubleTap();

    /// callback to handle drag gesture
    ///
    ///     @param x x position of the current drag point
    ///     @param y y position of the current drag point
    ///     @return true stop furthur processing the event, false otherwise
    virtual bool handleDrag(GestureState state, float x, float y);

    /// callback to handle pinch gesture
    ///
    ///     @param x1 x position of start point
    ///     @param y1 y position of start point
    ///     @param x2 x position of end point
    ///     @param y2 y position of end point
    ///     @return true stop furthur processing the event, false otherwise
    virtual bool handlePinch(GestureState state, float x1, float y1, float x2, float y2);

    std::string getIntentString(const std::string& name);
    std::shared_ptr<EngineContext> getEngineContext();

    friend class EngineContext;
private:
    // event processing
    void appCmd(int32_t cmd);
    bool inputKeyEvent(int action, int code);
    bool inputMotionEvent(int action);
    bool inputEvent(AInputEvent* event);

    static void onAppCmd(struct android_app* app, int32_t cmd);

    /// @return 0, the framework will continue handling the event
    /// @return 1, the framework will stop handling the event
    static int32_t onInputEvent(struct android_app* app, AInputEvent* event);

    bool updateFrame();

private:
    std::shared_ptr<EngineContext>                  mEngineContext;
    struct android_app*                             mApp;
    JNIEnv*                                         mJNIEnv;
    std::chrono::high_resolution_clock::time_point  mLastUpdated;
    bool                                            mFirstFrameUpdated;
    ndk_helper::DoubletapDetector                   mDoubleTapDetector;
    ndk_helper::PinchDetector                       mPinchDetector;
    ndk_helper::DragDetector                        mDragDetector;
};

} // namespace dzy

#endif
