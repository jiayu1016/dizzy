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

    void dumpParameter();

    friend class Render;
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

class Animation {
};

class SceneManager;
class EngineContext;
class Program;
class Node;
class Mesh;
class Material;
class Camera;
typedef std::vector<std::shared_ptr<Camera> >      CameraContainer;
typedef std::vector<std::shared_ptr<Light> >       LightContainer;
typedef std::vector<std::shared_ptr<Animation> >   AnimationContainer;
typedef std::vector<std::shared_ptr<Texture> >     TextureContainer;
typedef std::vector<std::shared_ptr<Material> >    MaterialContainer;
typedef std::vector<std::shared_ptr<Mesh> >        MeshContainer;
class Scene {
public:
    Scene();

    inline unsigned int getNumCameras() { return mCameras.size(); }
    inline unsigned int getNumLights() { return mLights.size(); }
    inline unsigned int getNumTextures() { return mTextures.size(); }
    inline unsigned int getNumAnimations() { return mAnimations.size(); }
    inline unsigned int getNumMaterials() { return mMaterials.size(); }
    inline unsigned int getNumMeshes() { return mMeshes.size(); }
    std::shared_ptr<Camera> getActiveCamera();

    bool atLeastOneMeshHasVertexPosition();
    bool atLeastOneMeshHasVertexColor();
    bool atLeastOneMeshHasNormal();

    std::shared_ptr<Node> getRootNode() { return mRootNode;}

    friend class SceneManager;
    friend class Render;
    friend class Program;
    friend class AIAdapter;
private:
    unsigned int            mFlags;
    CameraContainer         mCameras;
    LightContainer          mLights;
    TextureContainer        mTextures;
    AnimationContainer      mAnimations;
    MaterialContainer       mMaterials;
    MeshContainer           mMeshes;
    std::shared_ptr<Node>   mRootNode;

    // transient status for easy traversal
    glm::mat4           mCameraModelTransform;
    glm::mat4           mLightModelTransform;
};

class SceneManager : public Singleton<SceneManager> {
public:
    void addScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> getCurrentScene();
    void setCurrentScene(std::shared_ptr<Scene> scene);
    static std::shared_ptr<Scene> loadFile(
        std::shared_ptr<EngineContext> engineContext,
        const std::string &file);
    static std::shared_ptr<Scene> loadColladaAsset(
        std::shared_ptr<EngineContext> engineContext,
        const std::string &asset);

    friend class Singleton<SceneManager>;
private:    
    explicit SceneManager();

    std::vector<std::shared_ptr<Scene> >    mScenes;
    std::shared_ptr<Scene>                  mCurrentScene;
};

} // namespace dzy
#endif
