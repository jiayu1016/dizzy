#include <memory>
#include <string>
#include "log.h"
#include "engine_core.h"
#include "engine_context.h"
#include "scene_graph.h"
#include "mesh.h"
#include "scene.h"
#include "camera.h"

using namespace dzy;
using namespace std;

class SViewer : public EngineCore {
public:
    virtual bool create();
    virtual bool start();
    virtual bool update(long interval);
    virtual shared_ptr<Scene> getScene();
    virtual bool handlePinch(GestureState state, float x1, float y1, float x2, float y2);
    virtual bool handleDrag(GestureState state, float x, float y);
private:
    shared_ptr<Scene>   mScene;
    string              mSceneFileName;
    bool                mIsAsset;
    float               mScale;
    float               mLen;
    float               mDragStartX;
    float               mDragStartY;
};

bool SViewer::create() {
    mScale = 1.f;
    Log::setDebugSwitch(true);
    //Log::setFlag(Log::F_TRACE);
    //Log::setFlag(Log::F_GLES);
    //Log::setFlag(Log::F_EVENT);
    mSceneFileName = getIntentString("modelName");
    mIsAsset = getIntentBool("isAsset");
    return true;
}

bool SViewer::start() {
    shared_ptr<EngineContext> engineContext(getEngineContext());

    if (mIsAsset)
        mScene = Scene::loadColladaFromAsset(engineContext, mSceneFileName);
    else
        mScene = Scene::loadColladaFromFile(mSceneFileName);
    if (!mScene) return false;

    shared_ptr<Node> rootNode(mScene->getRootNode());
    rootNode->dumpHierarchy();
    return true;
}

bool SViewer::update(long interval) {
    return true;
}

shared_ptr<Scene> SViewer::getScene() {
    return mScene;
}

bool SViewer::handlePinch(GestureState state, float x1, float y1, float x2, float y2) {
    if (!mScene) return true;
    shared_ptr<Node> rootNode(mScene->getRootNode());
    if (state == EngineCore::GESTURE_PINCH_START) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        mLen = sqrt(dx * dx + dy * dy);
    } else {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = sqrt(dx * dx + dy * dy);
        mScale += (len - mLen) * 0.001f;
        if (mScale < 0.1f) mScale = 0.1f;
        if (mScale > 10.f) mScale = 10.f;
        mLen = len;
        rootNode->setLocalScale(1.f);
        rootNode->scale(mScale);
    }
    return true;
}

bool SViewer::handleDrag(GestureState state, float x, float y) {
    shared_ptr<Camera> camera(mScene->getActiveCamera());
    if (state == EngineCore::GESTURE_DRAG_START) {
        mDragStartX = x;
        mDragStartY = y;
    } else if (state == EngineCore::GESTURE_DRAG_MOVE) {
        float dx = x - mDragStartX;
        float dy = y - mDragStartY;
        mDragStartX = x;
        mDragStartY = y;
        camera->yaw(-dx / 2000.f * (float)M_PI);
        camera->pitch(-dy / 2000.f * (float)M_PI);
    }
    return true;
}

EngineCore * engine_main() {
    return new SViewer;
}
