#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>
#include "utils.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace dzy {

class Texture {
};

class SceneManager;
class EngineContext;
class Program;
class Node;
class Mesh;
class Material;
class Camera;
class Light;
class Animation;
typedef std::vector<std::shared_ptr<Camera> >      CameraContainer;
typedef std::vector<std::shared_ptr<Light> >       LightContainer;
typedef std::vector<std::shared_ptr<Animation> >   AnimationContainer;
typedef std::vector<std::shared_ptr<Texture> >     TextureContainer;
typedef std::vector<std::shared_ptr<Material> >    MaterialContainer;
typedef std::vector<std::shared_ptr<Mesh> >        MeshContainer;
class Scene {
public:
    Scene();
    ~Scene();

    inline unsigned int getNumCameras() { return mCameras.size(); }
    inline unsigned int getNumLights() { return mLights.size(); }
    inline unsigned int getNumTextures() { return mTextures.size(); }
    inline unsigned int getNumAnimations() { return mAnimations.size(); }
    inline unsigned int getNumMaterials() { return mMaterials.size(); }
    inline unsigned int getNumMeshes() { return mMeshes.size(); }
    std::shared_ptr<Camera>     getActiveCamera();
    std::shared_ptr<Camera>     getCamera(int idx);
    std::shared_ptr<Light>      getLight(int idx);
    std::shared_ptr<Animation>  getAnimation(int idx);

    bool atLeastOneMeshHasVertexPosition();
    bool atLeastOneMeshHasVertexColor();
    bool atLeastOneMeshHasNormal();

    std::shared_ptr<Node> getRootNode() { return mRootNode;}

    static std::shared_ptr<Scene> loadColladaFromFile(
        const std::string &file);
    static std::shared_ptr<Scene> loadColladaFromAsset(
        std::shared_ptr<EngineContext> engineContext,
        const std::string &asset);
    static std::shared_ptr<Scene> loadColladaFromMemory(
        const char *buf, int length, const std::string& fileName = "");

    friend class Render;
    friend class Program;
    friend class AIAdapter;
private:
    unsigned int            mFlags;
    CameraContainer         mCameras;
    int                     mActiveCamera;
    LightContainer          mLights;
    TextureContainer        mTextures;
    AnimationContainer      mAnimations;
    MaterialContainer       mMaterials;
    MeshContainer           mMeshes;
    std::shared_ptr<Node>   mRootNode;

    // transient status for easy traversal
    glm::mat4               mCameraModelTransform;
    glm::mat4               mLightModelTransform;
};

} // namespace dzy
#endif
