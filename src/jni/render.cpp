#include <memory>
#include <functional>
#include "log.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

bool Render::init() {
    bool success = false;
    Shader vtxShader(GL_VERTEX_SHADER);
    //success = vtxShader.compileFromMemory();
    Shader fragShader(GL_FRAGMENT_SHADER);
    //success = fragShader.compileFromMemory(const GLchar * source,const int32_t size);
    Program prog;
    //success = prog.link(vtxShader, fragShader);
    return true;
}

bool Render::release() {
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    NodeTree &tree = scene->getNodeTree();
    using namespace std::placeholders;
    tree.dfsTraversal(scene, bind(&Render::drawNode, this, _1, _2));
    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    ALOGD("Node %s has %d meshes", node->mName.c_str(), node->mMeshes.size());
    for (size_t i=0; i<node->mMeshes.size(); i++) {
        int meshIdx = node->mMeshes[i];
        ALOGD("meshIdx: %d", meshIdx);
        shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
        ALOGD("mesh name: %s", mesh->mName.c_str());
        mesh->draw(*this);
    }
}

} // namespace dzy
