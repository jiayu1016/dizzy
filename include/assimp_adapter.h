#ifndef ASSIMP_ADAPTER_H
#define ASSIMP_ADAPTER_H

#include <memory>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace dzy {

class AssetIOStream : public Assimp::IOStream
{
protected:
    // Used only by friend class AssetIOSystem
    AssetIOStream(void);

public:
    ~AssetIOStream(void);
    std::size_t Read(void* buffer, std::size_t size, std::size_t count);
    std::size_t Write(const void* buffer, std::size_t size, std::size_t count);
    aiReturn Seek(std::size_t offset, aiOrigin origin);
    std::size_t Tell() const;
    std::size_t FileSize() const;
    void Flush();

    friend class AssetIOSystem;

};

class AssetIOSystem : public Assimp::IOSystem
{
public:
    AssetIOSystem() {};
    ~AssetIOSystem() {};

    // Check whether a specific file exists
    bool Exists(const std::string& file) const { return true; }

    // Get the path delimiter character
    char GetOsSeparator() const { return '/'; }

    // Open a custom stream
    AssetIOStream* Open(const std::string& file, const std::string& mode) {
        return new AssetIOStream();
    }
    void Close(AssetIOStream* file) { delete file; }
};

class Camera;
class Light;
class Texture;
class Animation;
class Material;
class Mesh;
class Node;
class NodeTree;
class AIAdapter {
public:
    static std::shared_ptr<Camera>     typeCast(aiCamera *camera);
    static std::shared_ptr<Light>      typeCast(aiLight *light);
    static std::shared_ptr<Texture>    typeCast(aiTexture *texture);
    static std::shared_ptr<Animation>  typeCast(aiAnimation *animation);
    static std::shared_ptr<Material>   typeCast(aiMaterial *material);
    static std::shared_ptr<Mesh>       typeCast(aiMesh *mesh);
    static std::shared_ptr<Node>       typeCast(aiNode *node);
    static glm::vec3                   typeCast(const aiVector3D &vec3d);
    static glm::vec3                   typeCast(const aiColor3D &color3d);
    static glm::vec4                   typeCast(const aiColor4D &color4d);
    static glm::mat4                   typeCast(const aiMatrix4x4 &mat4);
    static std::string                 typeCast(const aiString &str);

    /// build scene graph
    ///
    ///     must be called AFTER the scene is built except scene graph
    ///     @param scene the scene that hosts the scene graph being built
    ///     @param root the root node of assimp Node
    static void buildSceneGraph(std::shared_ptr<Scene> scene, aiNode *aroot);
private:
    static void linkNode(std::shared_ptr<Scene> scene, std::shared_ptr<Node> node, aiNode *anode);
};

}

#endif
