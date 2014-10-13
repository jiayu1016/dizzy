// designed to hide data structures of assimp,  type conversion
// is implemented in overloaded functions in other files
#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <functional>
#include <algorithm>
#include "utils.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace dzy {

class AIAdapter;
class Scene;
class Render;

class Camera {
public:
    Camera();
    Camera(
        glm::vec3           position,
        glm::vec3           up,
        glm::vec3           lookAt,
        float               horizontalFOV,
        float               clipPlaneNear,
        float               clipPlaneFar,
        float               aspect);

    void        setAspect(float aspect);
    glm::mat4   getViewMatrix();
    glm::mat4   getProjMatrix();
    void        dumpParameter();

    friend class Render;
    friend class AIAdapter;
private:
    std::string         mName;
    glm::vec3           mPosition;
    glm::vec3           mUp;
    glm::vec3           mLookAt;
    float               mHorizontalFOV;
    float               mClipPlaneNear;
    float               mClipPlaneFar;
    float               mAspect;

};

class Light {
public:
    enum LightSourceType {
        LIGHT_SOURCE_UNDEFINED      = 0x0,
        LIGHT_SOURCE_DIRECTIONAL    = 0x1,
        LIGHT_SOURCE_POINT          = 0x2,
        LIGHT_SOURCE_SPOT           = 0x3,
    };
    Light();
    Light(
        LightSourceType type,
        float           attenuationConstant,
        float           attenuationLinear,
        float           attenuationQuadratic,
        float           angleInnerCone,
        float           angleOuterCone);

    friend class AIAdapter;
private:
    std::string         mName;
    LightSourceType     mType;
    glm::vec3           mPosition;
    glm::vec3           mDirection;
    float               mAttenuationConstant;
    float               mAttenuationLinear;
    float               mAttenuationQuadratic;
    glm::vec3           mColorDiffuse;
    glm::vec3           mColorSpecular;
    glm::vec3           mColorAmbient;
    float               mAngleInnerCone;
    float               mAngleOuterCone;

};

class Texture {
};

class Material {
public:
    enum MaterialType {
        COLOR_DIFFUSE,
        COLOR_SPECULAR,
        COLOR_AMBIENT,
    };
    bool get(MaterialType type, glm::vec3& color);

    friend class AIAdapter;
private:
    glm::vec3   mDiffuse;
    glm::vec3   mSpecular;
    glm::vec3   mAmbient;
};

class Animation {
};
class TriangleFace {
public:
    unsigned int mIndices[3];
};

/*
 * Used to hold raw buffer data of Mesh
 */
class MeshData {
public:
    enum MeshDataType {
        MESH_DATA_TYPE_NONE,
        MESH_DATA_TYPE_FLOAT,
        MESH_DATA_TYPE_INT,
        MESH_DATA_TYPE_UNSIGNED_SHORT,
        MESH_DATA_TYPE_FIXED16_16,
        MESH_DATA_TYPE_UNSIGNED_BYTE,
        MESH_DATA_TYPE_SHORT,
        MESH_DATA_TYPE_SHORT_NORM,
        MESH_DATA_TYPE_BYTE,
        MESH_DATA_TYPE_BYTE_NORM,
        MESH_DATA_TYPE_UNSIGNED_BYTE_NORM,
        MESH_DATA_TYPE_UNSIGNED_SHORT_NORM,
        MESH_DATA_TYPE_UNSIGNED_INT,
        MESH_DATA_TYPE_RGBA,
        MESH_DATA_TYPE_ARGB,
        MESH_DATA_TYPE_ABGR,
    };

    MeshData();
    inline bool empty() const { return mBuffer.get() == NULL; }
    void reset();
    void set(MeshDataType type, unsigned int numComponents,
        unsigned int stride, unsigned int numVertices,
        unsigned char *rawBuffer);
    inline unsigned int getBufSize() { return mBufSize; };
    inline unsigned int getBufStride() { return mStride; };
    inline unsigned int getNumComponents() { return mNumComponents; };

    void * getBuf();

private:
    MeshDataType                    mType;
    unsigned int                    mNumComponents;
    unsigned int                    mStride;
    unsigned int                    mBufSize;
    // c++11 shared_ptr doesn't support array, must explicitly set deleter
    std::shared_ptr<unsigned char>  mBuffer;
};

class Mesh {
public:
    enum {
        MAX_COLOR_SETS          = 0x8,
        MAX_TEXTURECOORDS       = 0x8,
    };

    enum PrimitiveType {
        PRIMITIVE_TYPE_TRIANGLES = 0,
        PRIMITIVE_TYPE_NUM
    };

    Mesh();

    bool            hasPositions() const;
    bool            hasFaces() const;
    bool            hasNormals() const;
    bool            hasTangentsAndBitangents() const;
    bool            hasVertexColors(unsigned int index) const;
    bool            hasVertexColors() const;
    bool            hasTextureCoords(unsigned int index) const;
    unsigned int    getNumUVChannels() const;
    unsigned int    getNumColorChannels() const;
    unsigned int    getNumVertices() const;
    unsigned int    getNumFaces() const;
    unsigned int    getNumIndices();
    unsigned int    getVertexNumComponent();
    unsigned int    getVertexBufSize();
    unsigned int    getVertexBufStride();
    void *          getVertexBuf();
    unsigned int    getIndexBufSize();
    void *          getIndexBuf();

    // default 3 float a float veretex
    void            dumpVertexBuf(int groupSize = 3);
    void            dumpIndexBuf(int groupSize = 3);

    friend class AIAdapter;
    friend class Render;
private:
    std::string                     mName;
    PrimitiveType                   mPrimitiveType;

    unsigned int                    mNumVertices;
    unsigned int                    mNumFaces;
    unsigned int                    mNumUVComponents[MAX_TEXTURECOORDS];

    MeshData                        mVertices;
    MeshData                        mNormals;
    MeshData                        mTangents;
    MeshData                        mBitangents;
    std::vector<TriangleFace>       mTriangleFaces;

    MeshData                        mColors         [MAX_COLOR_SETS];
    MeshData                        mTextureCoords  [MAX_TEXTURECOORDS];

    // A mesh use only ONE material, otherwise it is splitted to multiple meshes
    unsigned int                    mMaterialIndex;
};

class NodeTree;
class Node : public std::enable_shared_from_this<Node> {
public:
    Node() { }
    Node(const std::string& name) : mName(name) { }

    void addChild(std::shared_ptr<Node> node);
    //void setParent(std::shared_ptr<Node> parent);
    std::shared_ptr<Node> findNode(const std::string &name);

    friend class NodeTree;
    friend class AIAdapter;
    friend class Render;
private:

    std::string                             mName;
    glm::mat4                               mTransformation;
    std::weak_ptr<Node>                     mParent;
    std::vector<std::shared_ptr<Node> >     mChildren;
    std::vector<int>                        mMeshes;

};

class NodeTree {
public:
    typedef std::function<void(std::shared_ptr<Scene>, std::shared_ptr<Node>)> VisitFunc;
    inline void setRoot(std::shared_ptr<Node> root) {
        mRoot = root;
    }
    void dfsTraversal(std::shared_ptr<Scene> scene, VisitFunc visit);

    friend class AIAdapter;
private:
    void dfsTraversal(std::shared_ptr<Scene> scene, std::shared_ptr<Node> node, VisitFunc visit);

    std::shared_ptr<Node> mRoot;
};

class MatrixStack {
public:
    MatrixStack();
    void push(glm::mat4 &matrix);
    void pop();
    glm::mat4 top();

private:
    std::stack<glm::mat4> mProduct;
};

class SceneManager;
class AppContext;
class Program;
typedef std::vector<std::shared_ptr<Camera> >      CameraContainer;
typedef std::vector<std::shared_ptr<Light> >       LightContainer;
typedef std::vector<std::shared_ptr<Animation> >   AnimationContainer;
typedef std::vector<std::shared_ptr<Texture> >     TextureContainer;
typedef std::vector<std::shared_ptr<Material> >    MaterialContainer;
typedef std::vector<std::shared_ptr<Mesh> >        MeshContainer;
class Scene {
public:
    Scene();
    virtual bool loadColladaAsset(std::shared_ptr<AppContext> appContext, const std::string &asset);
    virtual bool load(std::shared_ptr<AppContext> appContext, const std::string &file);
    virtual bool listAssetFiles(std::shared_ptr<AppContext> appContext,
        const std::string &dir);

    inline unsigned int getNumCameras() { return mCameras.size(); }
    inline unsigned int getNumLights() { return mLights.size(); }
    inline unsigned int getNumTextures() { return mTextures.size(); }
    inline unsigned int getNumAnimations() { return mAnimations.size(); }
    inline unsigned int getNumMaterials() { return mMaterials.size(); }
    inline unsigned int getNumMeshes() { return mMeshes.size(); }
    //inline size_t getNumNodes() { return mNodeTree.getNumNodes(); }
    std::shared_ptr<Camera> getActiveCamera();

    bool atLeastOneMeshHasVertexPosition();
    bool atLeastOneMeshHasVertexColor();

    inline NodeTree& getNodeTree() { return mNodeTree;}

    friend class SceneManager;
    friend class Render;
    friend class Program;
    friend class NodeTree;
private:
    unsigned int        mFlags;
    CameraContainer     mCameras;
    LightContainer      mLights;
    TextureContainer    mTextures;
    AnimationContainer  mAnimations;
    MaterialContainer   mMaterials;
    MeshContainer       mMeshes;
    NodeTree            mNodeTree;

    // transient status for easy traversal
    unsigned int        mNodeDepth;
    MatrixStack         mMatrixStack;
};

class SceneManager : public Singleton<SceneManager> {
public:
    enum SceneType {
        SCENE_TYPE_FLAT,
    };
    ~SceneManager();
    std::shared_ptr<Scene> createScene(SceneType);
    void addScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> getCurrentScene();

    friend class Singleton<SceneManager>;
private:    
    explicit SceneManager();

    std::vector<std::shared_ptr<Scene> >    mScenes;
    std::shared_ptr<Scene>                  mCurrentScene;
};

} // namespace dzy
#endif
