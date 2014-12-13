#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <GLES3/gl3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "nameobj.h"
#include "transform.h"

namespace dzy {

class Program;
class Render;
class Scene;
class Mesh;
class Node;
class Material;
class Camera;
class Light;
class NodeAnim;
/// Base class for "element" in the scene graph
class NodeObj : public NameObj, public std::enable_shared_from_this<NodeObj> {
public:
    enum Flags {
        F_UPDATE_TRANSFORM = 0x1,
    };
    NodeObj(const std::string& name);
    virtual ~NodeObj();

    glm::quat getWorldRotation();
    glm::vec3 getWorldTranslation();
    glm::vec3 getWorldScale();
    Transform getWorldTransform();
    glm::quat getLocalRotation();
    void setLocalRotation(const glm::quat& quaternion);
    void setLocalRotation(float w, float x, float y, float z);
    glm::vec3 getLocalScale();
    void setLocalScale(float scale);
    void setLocalScale(float x, float y, float z);
    void setLocalScale(const glm::vec3& scale);
    glm::vec3 getLocalTranslation();
    void setLocalTranslation(const glm::vec3& translation);
    void setLocalTranslation(float x, float y, float z);
    void setLocalTransform(const Transform& transform);
    Transform getLocalTransform();
    NodeObj& translate(float x, float y, float z);
    NodeObj& translate(const glm::vec3& offset);
    NodeObj& scale(float s);
    NodeObj& scale(float x, float y, float z);
    NodeObj& scale(const glm::vec3& v);
    NodeObj& rotate(const glm::quat& rotation);
    NodeObj& rotate(float axisX, float axisY, float axisZ);

    /// get parent of this node
    ///
    ///     @return the parent of this node, null if this is the root node
    std::shared_ptr<Node> getParent();

    /// set parent of this node
    ///
    ///     simply set parent of the current node, will not
    ///     add this node to the new parent's children list
    void setParent(std::shared_ptr<Node> parent);

    /// check if the node use auto program
    bool isAutoProgram();

    /// set the program that is attached to this node
    ///
    ///     @param program the program to be attached to this node
    void setProgram(std::shared_ptr<Program> program);

    /// get the program that is attached to this node
    ///
    ///     @return the current program attached to this node
    std::shared_ptr<Program> getProgram();

    /// get the current program attached to this node
    ///
    ///     if using auto program, the material and mesh are used to
    ///     generate program
    ///
    ///     @param material the material used to generate program
    ///     @param hasLight the scene has light
    ///     @param mesh the mesh used to generate program
    ///     @return the current program attached to this node
    std::shared_ptr<Program> getProgram(
        std::shared_ptr<Material> material,
        bool hasLight,
        std::shared_ptr<Mesh> mesh);

    void setMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> getMaterial();

    // TODO: support multiple lights
    void setLight(std::shared_ptr<Light> light);
    std::shared_ptr<Light> getLight();

    void setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getCamera();

    void setAnimation(std::shared_ptr<NodeAnim> nodeAnim);
    std::shared_ptr<NodeAnim> getAnimation();
    void updateAnimation(double timeStamp);

    /// Recursively draw the node and it's children
    virtual void draw(Render &render,
        std::shared_ptr<Scene> scene, double timeStamp) = 0;

    friend class AIAdapter;
    friend class Render;
    friend class Node;
protected:
    void doUpdateTransform();
    void updateWorldTransform();
    virtual void setUpdateFlag();

protected:
    Transform                               mLocalTransform;
    Transform                               mWorldTransform;
    int                                     mUpdateFlags;
    std::weak_ptr<Node>                     mParent;
    bool                                    mUseAutoProgram;
    // make sure program released on-time, to avoid delete program
    // after egl context has been released.
    std::weak_ptr<Program>                  mProgram;
    std::shared_ptr<Material>               mMaterial;
    std::shared_ptr<Light>                  mLight;
    std::shared_ptr<Camera>                 mCamera;
    std::shared_ptr<NodeAnim>               mAnimation;
};

class Node : public NodeObj {
public:
    typedef std::function<void(std::shared_ptr<Scene>, std::shared_ptr<NodeObj>)> VisitSceneFunc;
    typedef std::function<void(std::shared_ptr<NodeObj>)> VisitFunc;

    Node(const std::string& name);
    ~Node();

    /// attach a child node into the scene graph
    ///
    ///     @param childNode the child node to attach
    void attachChild(std::shared_ptr<NodeObj> childNode);

    /// returns a child at a given index
    std::shared_ptr<NodeObj> getChild(int idx);

    /// returns a child match the given name, search recursively from current node
    std::shared_ptr<NodeObj> getChild(const std::string &name);

    /// depth first traversal of the scene graph(tree)
    ///
    ///     function template to do dfs traversal, starting from the current node,
    ///     not from the root of the whole scene graph
    ///
    ///     @param scene the scene that hosts the scene graph
    ///     @param visit the functor gets called whenever a node is visited
    void depthFirstTraversal(std::shared_ptr<Scene> scene, VisitSceneFunc visit);

    /// depth first traversal of the scene graph(tree)
    ///
    ///     function template to do dfs traversal, starting from the current node,
    ///     not from the root of the whole scene graph
    ///
    ///     @param visit the functor gets called whenever a node is visited
    void depthFirstTraversal(VisitFunc visit);

    virtual void draw(Render &render,
        std::shared_ptr<Scene> scene, double timeStamp);

    /// dump the scene graph hierarchy starting from the current node.
    void dumpHierarchy(Log::Flag f = Log::F_ALWAYS);

protected:
    virtual void setUpdateFlag();

protected:
    std::vector<std::shared_ptr<NodeObj> >  mChildren;
};

class Geometry : public NodeObj {
public:
    Geometry(std::shared_ptr<Mesh> mesh);
    Geometry(const std::string& name, std::shared_ptr<Mesh> mesh);
    ~Geometry();

    virtual void draw(Render &render,
        std::shared_ptr<Scene> scene, double timeStamp);

    std::shared_ptr<Mesh> getMesh();

protected:
    bool updateBufferObject();

protected:
    // one on one mapping between Geometry and Mesh
    std::shared_ptr<Mesh>       mMesh;
    // one vertex and index buffer object per Geometry
    // logically BO handles should be put in Mesh class,
    // but I prefer not to have Mesh class depend on gl
    GLuint                      mVertexBO;
    GLuint                      mIndexBO;
    bool                        mBOUpdated;
};

}

#endif
