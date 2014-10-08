#include <memory>
#include <functional>
#include "log.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

bool Render::init() {
    return true;
}

bool Render::release() {
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    NodeTree &tree = scene->getNodeTree();
    using namespace std::placeholders;
    tree.dfsTraversal(bind(&Render::drawNode, this, _1));
    return true;
}

void Render::drawNode(std::shared_ptr<Node> node) {
    ALOGD("Node: %s", node->mName.c_str());
}

} // namespace dzy
