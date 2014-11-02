#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <GLES3/gl3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace dzy {

class Program;
class Render;
class Scene;
class Mesh;
class Node : public std::enable_shared_from_this<Node> {
public:
    typedef std::function<void(std::shared_ptr<Scene>, std::shared_ptr<Node>)> VisitSceneFunc;
    typedef std::function<void(std::shared_ptr<Node>)> VisitFunc;

    Node();
    Node(const std::string& name);

    /// attach a child node into the scene graph
    ///
    ///     @param childNode the child node to attach
    void attachChild(std::shared_ptr<Node> childNode);

    /// get parent node
    std::shared_ptr<Node> getParent();

    /// find a node by name
    std::shared_ptr<Node> findNode(const std::string &name);

    /// dfs traversal of the scene graph(tree)
    ///
    ///     function template to do dfs traversal, starting from the current node,
    ///     not from the root of the whole scene graph
    ///
    ///     @param scene the scene that hosts the scene graph
    ///     @param visit the functor gets called whenever a node is visited
    void dfsTraversal(std::shared_ptr<Scene> scene, VisitSceneFunc visit);

    /// dfs traversal of the scene graph(tree)
    ///
    ///     function template to do dfs traversal, starting from the current node,
    ///     not from the root of the whole scene graph
    ///
    ///     @param visit the functor gets called whenever a node is visited
    void dfsTraversal(VisitFunc visit);

    /// check if the node use auto program
    bool isAutoProgram();

    /// set the program that is attached to this node
    ///
    ///     @param program the program to be attached to this node
    void setProgram(std::shared_ptr<Program> program);

    /// get the current program attached to this node
    ///
    ///     @return the current program attached to this node
    std::shared_ptr<Program> getProgram();

    /// one time initialization of the node
    virtual bool initGpuData();

    /// Recursively draw the node and it's children
    virtual void draw(Render &render, std::shared_ptr<Scene> scene);

    void resetTransform();
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);
    void rotate(float radian, float axisX, float axisY, float axisZ);

    void            setName(std::string name);
    std::string     getName();

    /// dump the scene graph hierarchy starting from the current node.
    void dumpHierarchy();

    friend class AIAdapter;
    friend class Render;
protected:
    void dump(int depth);

    std::string                             mName;
    glm::mat4                               mTransformation;
    std::weak_ptr<Node>                     mParent;
    std::vector<std::shared_ptr<Node> >     mChildren;
    bool                                    mUseAutoProgram;
    std::shared_ptr<Program>                mProgram;
    static int                              mMonoCount;
};

class GeoNode : public Node {
public:
    GeoNode(std::shared_ptr<Mesh> mesh);

    virtual bool initGpuData();
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
