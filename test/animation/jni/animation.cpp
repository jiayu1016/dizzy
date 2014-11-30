#include <memory>
#include <string>
#include <cmath>
#include "log.h"
#include "engine_core.h"
#include "engine_context.h"
#include "render.h"
#include "scene_graph.h"
#include "mesh.h"
#include "scene.h"

using namespace dzy;
using namespace std;

class AnimationApp : public EngineCore {
public:
    virtual bool start();
    virtual bool update(long interval);
    virtual shared_ptr<Scene> getScene();
    virtual bool handlePinch(GestureState state, float x1, float y1, float x2, float y2);
private:
    shared_ptr<Scene>   mScene;
    float               mAngle;
    float               mTranslate;
    float               mScale;
    float               mLen;
};

bool AnimationApp::start() {
    mScale = 1.f;

    shared_ptr<EngineContext> engineContext(getEngineContext());
    mScene = Scene::loadColladaAsset(engineContext, "4meshes.dae");
    if (!mScene) {
        ALOGE("failed to load collada scene");
        return false;
    }

    shared_ptr<Node> rootNode(mScene->getRootNode());
    if (!rootNode) return false;

    shared_ptr<Node> coneNode = dynamic_pointer_cast<Node>(rootNode->getChild("Cone"));
    shared_ptr<NodeObj> torusNode = rootNode->getChild("Torus");
    if (coneNode && torusNode)
        coneNode->attachChild(torusNode);

    shared_ptr<Node> sphereNode = dynamic_pointer_cast<Node>(rootNode->getChild("Sphere"));
    if (coneNode && sphereNode)
        coneNode->attachChild(sphereNode);

    rootNode->dumpHierarchy();
    return true;
}

bool AnimationApp::update(long interval) {
    shared_ptr<Node> rootNode(mScene->getRootNode());
    if (!rootNode) return false;

    shared_ptr<NodeObj> torusNode(rootNode->getChild("Torus"));
    if (torusNode)
        torusNode->rotate(interval / 1000.0f * 3.1415927, 0.f, 1.f, 0.f);

    shared_ptr<NodeObj> coneNode = rootNode->getChild("Cone");
    if (coneNode) {
        coneNode->resetTransform();
        mTranslate += interval / 1000.f;
        if (mTranslate > 7.f)
            mTranslate = 0.f;
        coneNode->translate(mTranslate, 0.f, 0.f);
    }

    shared_ptr<NodeObj> sphereNode = rootNode->getChild("Sphere");
    if (sphereNode) {
        sphereNode->resetTransform();
        mAngle += interval / 1000.0f * 3.1415927;
        const float radius = 3.f;
        sphereNode->translate(radius * cos(mAngle), radius * sin(mAngle), 0.f);
        sphereNode->rotate(interval / 1000.0f * 3.1415927, 0.f, 0.f, 1.f);
    }
    return true;
}

shared_ptr<Scene> AnimationApp::getScene() {
    return mScene;
}

bool AnimationApp::handlePinch(GestureState state, float x1, float y1, float x2, float y2) {
    if (!mScene) return true;
    shared_ptr<Node> rootNode(mScene->getRootNode());
    if (!rootNode) return true;
    if (state == EngineCore::GESTURE_PINCH_START) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        mLen = sqrt(dx * dx + dy * dy);
    } else {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = sqrt(dx * dx + dy * dy);
        mScale += (len - mLen) * 0.01f;
        mLen = len;
        rootNode->resetTransform();
        rootNode->scale(mScale, mScale, mScale);
    }
    return true;
}

EngineCore * engine_main() {
    return new AnimationApp;
}
