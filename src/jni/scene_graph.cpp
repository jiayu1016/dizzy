#include <algorithm>
#include <sstream>
#include <functional>
#include <queue>
#include <stack>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include "log.h"
#include "program.h"
#include "scene.h"
#include "render.h"
#include "mesh.h"
#include "material.h"
#include "scene_graph.h"

using namespace std;

namespace dzy {

int NodeObj::mMonoCount = 0;

NodeObj::NodeObj(const string& name)
    : NameObj(name)
    , mUseAutoProgram(true)
    , mInitialized(false) {
    mMonoCount++;
}

NodeObj::~NodeObj() {
    ALOGV("NodeObj::~NodeObj(), %s", getName().c_str());
}

void NodeObj::resetTransform() {
    mTransformation = glm::mat4(1.0f);
}

void NodeObj::translate(float x, float y, float z) {
    glm::vec3 v(x, y, z);
    mTransformation = glm::translate(mTransformation, v);
}

void NodeObj::scale(float x, float y, float z) {
    glm::vec3 v(x, y, z);
    mTransformation = glm::scale(mTransformation, v);
}

void NodeObj::rotate(float radian, float axisX, float axisY, float axisZ) {
    glm::vec3 axis(axisX, axisY, axisZ);
    mTransformation = glm::rotate(mTransformation, radian, axis);
}

shared_ptr<Node> NodeObj::getParent() {
    return mParent.lock();
}

void NodeObj::setParent(shared_ptr<Node> parent) {
    mParent = parent;
}

bool NodeObj::isAutoProgram() {
    return mUseAutoProgram;
}

void NodeObj::setProgram(shared_ptr<Program> program) {
    mUseAutoProgram = false;
    mProgram = program;
}

shared_ptr<Program> NodeObj::getProgram() {
    return mProgram;
}

shared_ptr<Program> NodeObj::getProgram(
    shared_ptr<Material> material, bool hasLight, shared_ptr<Mesh> mesh) {
    if (!mProgram) {
        if (mUseAutoProgram) {
            mProgram = ProgramManager::get()->getCompatibleProgram(material, hasLight, mesh);
        } else {
            ALOGE("No shader program found");
        }
    }
    return mProgram;
}

void NodeObj::setMaterial(shared_ptr<Material> material) {
    mMaterial = material;
}

shared_ptr<Material> NodeObj::getMaterial() {
    return mMaterial;
}

void NodeObj::setLight(shared_ptr<Light> light) {
    mLight = light;
}

shared_ptr<Light> NodeObj::getLight() {
    return mLight;
}

void NodeObj::setCamera(shared_ptr<Camera> camera) {
    mCamera = camera;
}

shared_ptr<Camera> NodeObj::getCamera() {
    return mCamera;
}

bool NodeObj::isInitialized() {
    return mInitialized;
}

void NodeObj::setInitialized() {
    mInitialized = true;
}

Node::Node(const string& name)
    : NodeObj(name) {
}

Node::~Node() {
    ALOGV("Node::~Node(), %s", getName().c_str());
}

void Node::attachChild(shared_ptr<NodeObj> childNode) {
    // no duplicate Node in children list
    bool exist = false;
    for (auto iter = mChildren.begin(); iter != mChildren.end(); iter++) {
        if (*iter == childNode) {
            exist = true;
            ALOGW("avoid inserting duplicate Node");
            break;
        }
    }

    if (!exist) {
        mChildren.push_back(childNode);
        shared_ptr<Node> oldParent(childNode->getParent());
        childNode->setParent(shared_from_this());
        if (oldParent) {
            // remove childNode from oldParent
            oldParent->mChildren.erase(remove(
                oldParent->mChildren.begin(), oldParent->mChildren.end(), childNode),
                oldParent->mChildren.end());
        }
    }
}

shared_ptr<NodeObj> Node::getChild(int idx) {
    return mChildren[idx];
}

shared_ptr<NodeObj> Node::getChild(const string &name) {
    queue<shared_ptr<NodeObj> > q;
    q.push(shared_from_this());
    while(!q.empty()) {
        shared_ptr<NodeObj> nodeObj(q.front());
        q.pop();
        if (nodeObj->getName() == name) return nodeObj;
        shared_ptr<Node> node = dynamic_pointer_cast<Node>(nodeObj);
        if (node) {
            for (size_t i = 0; i < node->mChildren.size(); i++) {
                q.push(node->mChildren[i]);
            }
        }
    }
    return nullptr;
}

void Node::depthFirstTraversal(shared_ptr<Scene> scene, VisitSceneFunc visit) {
    stack<shared_ptr<NodeObj> > stk;
    stk.push(shared_from_this());
    while (!stk.empty()) {
        shared_ptr<NodeObj> nodeObj = stk.top();
        stk.pop();
        visit(scene, nodeObj);
        shared_ptr<Node> node = dynamic_pointer_cast<Node>(nodeObj);
        if (!node) {
            for (auto it = node->mChildren.rbegin(); it != node->mChildren.rend(); ++it) {
                stk.push(*it);
            }
        }
    }
}

void Node::depthFirstTraversal(VisitFunc visit) {
    stack<shared_ptr<NodeObj> > stk;
    stk.push(shared_from_this());
    while (!stk.empty()) {
        shared_ptr<NodeObj> nodeObj = stk.top();
        stk.pop();
        visit(nodeObj);
        shared_ptr<Node> node = dynamic_pointer_cast<Node>(nodeObj);
        if (node) {
            for (auto it = node->mChildren.rbegin(); it != node->mChildren.rend(); ++it) {
                stk.push(*it);
            }
        }
    }
}

bool Node::initGpuData() {
    setInitialized();
    return true;
}

void Node::draw(Render &render, shared_ptr<Scene> scene) {
    if (!isInitialized()) initGpuData();
    render.drawNode(scene, shared_from_this());
    std::for_each(mChildren.begin(), mChildren.end(), [&] (shared_ptr<NodeObj> c) {
        c->draw(render, scene);
    });
}

void Node::dumpHierarchy() {
    typedef pair<shared_ptr<NodeObj>, int> STACK_ELEM;
    stack<STACK_ELEM> stk;
    stk.push(make_pair(shared_from_this(), 0));
    while (!stk.empty()) {
        STACK_ELEM elem = stk.top();
        stk.pop();
        shared_ptr<NodeObj> nodeObj = elem.first;
        int depth = elem.second;
        ostringstream os;
        for (int i=0; i<depth; i++) os << "    ";
        os << "%d:%s";
        PRINT(os.str().c_str(), depth, nodeObj->getName().c_str());
        shared_ptr<Node> node = dynamic_pointer_cast<Node>(nodeObj);
        if (node) {
            for (auto it = node->mChildren.rbegin(); it != node->mChildren.rend(); ++it) {
                stk.push(make_pair(*it, depth+1));
            }
        }
    }
}

Geometry::Geometry(shared_ptr<Mesh> mesh)
    : NodeObj(string("Geometry-") + mesh->getName())
    , mMesh(mesh) {
}

Geometry::Geometry(const string& name, shared_ptr<Mesh> mesh)
    : NodeObj(name)
    , mMesh(mesh) {
}

Geometry::~Geometry() {
    ALOGV("Geometry::~Geometry(), %s", getName().c_str());
}

bool Geometry::initGpuData() {
    if (!mMesh) {
        ALOGE("One Geometry must attach one Mesh");
        return false;
    }

    // Load vertex and index data into buffer object
    glGenBuffers(1, &mVertexBO);
    glGenBuffers(1, &mIndexBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBO);
    glBufferData(GL_ARRAY_BUFFER, mMesh->getVertexBufSize(),
        mMesh->getVertexBuf(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mMesh->getIndexBufSize(),
        mMesh->getIndexBuf(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    setInitialized();
    return true;
}

void Geometry::draw(Render &render, shared_ptr<Scene> scene) {
    if (!isInitialized()) initGpuData();
    if (render.drawGeometry(scene, shared_from_this()))
        // program attached to Geometry node only when drawGeometry returns true
        render.drawMesh(scene, mMesh, getProgram(), mVertexBO, mIndexBO);
}

std::shared_ptr<Mesh> Geometry::getMesh() {
    return mMesh;
}

} //namespace
