#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <GLES3/gl3.h>

namespace dzy {

class Program;
class Render;
class Scene;
class Mesh;
class Node : public std::enable_shared_from_this<Node> {
public:
    typedef std::function<void(std::shared_ptr<Scene>, std::shared_ptr<Node>)> VisitFunc;

    Node();
    Node(const std::string& name) : mName(name) { }

    /// attach a child node into the scene graph
    ///
    ///     @param childNode the child node to attach
    void attachChild(std::shared_ptr<Node> childNode);
    std::shared_ptr<Node> getParent();
    std::shared_ptr<Node> findNode(const std::string &name);
    void dfsTraversal(std::shared_ptr<Scene> scene, VisitFunc visit);

    /// check if the node use auto program
    bool isAutoProgram();
    void setAutoProgram();
    /// get the current program attached to this node
    ///
    ///     @return the current program attached to this node
    std::shared_ptr<Program> getProgram();

    /// one time initialization of the node
    virtual bool init();

    /// Recursively draw the node and it's children
    virtual void draw(Render &render, std::shared_ptr<Scene> scene);

    friend class AIAdapter;
    friend class Render;
protected:
    std::string                             mName;
    glm::mat4                               mTransformation;
    std::weak_ptr<Node>                     mParent;
    std::vector<std::shared_ptr<Node> >     mChildren;
    bool                                    mUseAutoProgram;
    std::shared_ptr<Program>                mProgram;
};

class GeoNode : public Node {
public:
    GeoNode(std::shared_ptr<Mesh> mesh) : mMesh(mesh) {};

    virtual bool init();
    virtual void draw(Render &render, std::shared_ptr<Scene> scene);
protected:
    // one on one mapping between GeoNode and Mesh
    std::shared_ptr<Mesh>     mMesh;
    // one vertex and index buffer object per GeoNode
    // logically BO handles should be put in Mesh class,
    // but I prefer not to have Mesh class depend on gl
    GLuint  mVertexBO;
    GLuint  mIndexBO;
};

}

#endif