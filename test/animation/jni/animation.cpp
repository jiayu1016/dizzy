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
#include "camera.h"

using namespace dzy;
using namespace std;

class AnimationApp : public EngineCore {
public:
    virtual bool start();
    virtual bool update(long interval);
    virtual shared_ptr<Scene> getScene();
    virtual bool handlePinch(GestureState state, float x1, float y1, float x2, float y2);
    virtual bool handleDrag(GestureState state, float x, float y);
private:
    shared_ptr<Scene>   mScene;
    float               mScale;
    float               mLen;
    float               mDragStartX;
    float               mDragStartY;
};

bool AnimationApp::start() {
    mScale = 1.f;
    mScene.reset(new Scene);

    shared_ptr<Node> rootNode(mScene->getRootNode());

    shared_ptr<CubeMesh> cube3Mesh(new CubeMesh("cube3Mesh"));
    shared_ptr<Geometry> cube3Geom(new Geometry("cube3Geom", cube3Mesh));
    cube3Geom->scale(0.2);
    cube3Geom->translate(0, 0.5, 0);

    shared_ptr<CubeMesh> cube2Mesh(new CubeMesh("cube2Mesh"));
    shared_ptr<Geometry> cube2Geom(new Geometry("cube2Geom", cube2Mesh));
    cube2Geom->scale(0.5);
    shared_ptr<Node> cube2Node(new Node("cube2Node"));
    cube2Node->attachChild(cube2Geom);
    cube2Node->attachChild(cube3Geom);
    cube2Node->translate(0, 2, 0);

    shared_ptr<CubeMesh> cube1Mesh(new CubeMesh("cube1Mesh"));
    shared_ptr<Geometry> cube1Geom(new Geometry("cube1Geom", cube1Mesh));
    shared_ptr<Node> cube1Node(new Node("cube1Node"));
    cube1Node->attachChild(cube2Node);

    shared_ptr<PyramidMesh> pyramidMesh(new PyramidMesh("pyramid_mesh"));
    shared_ptr<Geometry> pyramidNode(new Geometry("pyramid", pyramidMesh));
    pyramidNode->translate(3, 0, 0);
    pyramidNode->scale(2);

    rootNode->attachChild(cube1Geom);
    rootNode->attachChild(cube1Node);
    rootNode->attachChild(pyramidNode);

    rootNode->dumpHierarchy();

    return true;
}

bool AnimationApp::update(long interval) {
    shared_ptr<Node> rootNode(mScene->getRootNode());

    shared_ptr<Geometry> earth = dynamic_pointer_cast<Geometry>(rootNode->getChild("cube2Geom"));
    if (earth)
        earth->rotate(interval / 1000.f, 0, 0);

    shared_ptr<Geometry> moon = dynamic_pointer_cast<Geometry>(rootNode->getChild("cube3Geom"));
    if (moon)
        moon->rotate(0, interval / 1000.f, 0);

    shared_ptr<Node> earthSystem = dynamic_pointer_cast<Node>(rootNode->getChild("cube2Node"));
    if (earthSystem)
        earthSystem->rotate(0, 0, interval / 200.f);

    shared_ptr<Node> solarSystem = dynamic_pointer_cast<Node>(rootNode->getChild("cube1Node"));
    if (solarSystem)
        solarSystem->rotate(0, 0, interval / 1000.f);

    return true;
}

shared_ptr<Scene> AnimationApp::getScene() {
    return mScene;
}

bool AnimationApp::handlePinch(GestureState state, float x1, float y1, float x2, float y2) {
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
        rootNode->scale(mScale, mScale, mScale);
    }
    return true;
}

bool AnimationApp::handleDrag(GestureState state, float x, float y) {
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
    return new AnimationApp;
}
