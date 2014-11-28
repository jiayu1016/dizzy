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
    virtual bool initView();
    virtual bool releaseView();
    virtual bool update(long interval);
    virtual shared_ptr<Scene> getScene();
private:
    shared_ptr<Scene> mScene;
    float mAngle;
    float mTranslate;
};

bool AnimationApp::initView() {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    engineContext->listAssetFiles("");

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

bool AnimationApp::releaseView() {
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

EngineCore * engine_main() {
    return new AnimationApp;
}
