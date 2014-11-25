#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <vector>
#include <memory>
#include <GLES3/gl3.h>
#include "utils.h"

class AAssetManager;

namespace dzy {

class Shader {
public:
    enum ShaderType {
        Vertex,
        Fragment,
        Geometry
    };

    Shader(ShaderType type);
    ~Shader();
    GLenum getShaderType();
    GLuint getShaderID();

    bool compileFromMemory(std::vector<uint8_t>& data);
    bool compileFromMemory(const GLchar *source, const int32_t size);
    bool compileFromFile(const char *strFileName);
    bool compileFromAsset(AAssetManager *assetManager, const std::string &assetFile);

private:
    bool        mInitialized;
    GLuint      mShaderId;
    GLenum      mShaderType;
};

#define STORE_CHECK_ATTRIB_LOC(ATTRIB) do {                             \
        GLint attribLoc = glGetAttribLocation(mProgramId, ATTRIB);      \
        if (attribLoc == -1) {                                          \
            break;                                                      \
        }                                                               \
        mLocations[ATTRIB] = attribLoc;                                 \
    } while(0)

#define STORE_CHECK_UNIFORM_LOC(UNIFORM) do {                           \
        GLint uniformLoc = glGetUniformLocation(mProgramId, UNIFORM);   \
        if (uniformLoc == -1) {                                         \
            break;                                                      \
        }                                                               \
        mLocations[UNIFORM] = uniformLoc;                               \
    } while(0)

class Scene;
class Camera;
class Light;
class Material;
class Mesh;
class Program {
public:
    enum Requirement {
        REQUIRE_VERTEX_COLOR,
        REQUIRE_VERTEX_NORMAL,
        REQUIRE_MATERIAL,
        REQUIRE_LIGHT,
    };

    Program();
    virtual ~Program();

    GLuint getId() const { return mProgramId; }
    bool isValid() const { return mLinked; }
    // bind program
    void use();
    bool link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader);
    /// store attribute and uniform location after binding
    virtual bool storeLocation();
    GLint getLocation(const char* name);
    void setRequirement(
        bool requireVertexColor,
        bool requireVertexNormal,
        bool requireMaterial,
        bool requireLight);
    bool hasRequirement(Requirement requirement);

    /// upload data to gpu
    ///
    ///     data including material, light, transform and anything else except mesh
    ///
    ///     @param camera current using camera
    ///     @param light
    ///     @param material current using material
    ///     @param world world transfomation matrix to upload
    ///     @param view view transformation matrix to upload
    ///     @param proj projection transformation matrix to upload
    virtual bool uploadData(
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Light> light,
        std::shared_ptr<Material> material,
        glm::mat4& world,
        glm::mat4& view,
        glm::mat4& proj);

    /// update mesh data
    ///
    ///     mesh data including all datas that are put in vbo,
    ///     vbo should  be kept  until glDrawXXX gets called.
    ///
    ///     @mesh the mesh beging drawn
    ///     @vbo vertex buffer object that hold the vertex data structures of arrays
    virtual bool updateMeshData(std::shared_ptr<Mesh> mesh, GLuint vbo);

    friend class Shader;

protected:
    bool                                        mLinked;
    GLuint                                      mProgramId;
    std::vector<std::shared_ptr<Shader> >       mShaders;
    std::map<std::string, GLint>                mLocations;
    int                                         mRequirement;
};

///////////////////////////////////////////
//            built-in programs
///////////////////////////////////////////
class Program000 : public Program {
public:
    Program000();
    virtual bool storeLocation();
    virtual bool uploadData(
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Light> light,
        std::shared_ptr<Material> material,
        glm::mat4& world,
        glm::mat4& view,
        glm::mat4& proj);
    virtual bool updateMeshData(std::shared_ptr<Mesh> mesh, GLuint vbo);
};

class Program010 : public Program {
public:
    Program010();
    virtual bool storeLocation();
    virtual bool uploadData(
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Light> light,
        std::shared_ptr<Material> material,
        glm::mat4& world,
        glm::mat4& view,
        glm::mat4& proj);
    virtual bool updateMeshData(std::shared_ptr<Mesh> mesh, GLuint vbo);
};

class Program020 : public Program {
public:
    Program020();
    virtual bool storeLocation();
    virtual bool uploadData(
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Light> light,
        std::shared_ptr<Material> material,
        glm::mat4& world,
        glm::mat4& view,
        glm::mat4& proj);
    virtual bool updateMeshData(std::shared_ptr<Mesh> mesh, GLuint vbo);
};

class Program100 : public Program {
public:
    Program100();
    virtual bool storeLocation();
    virtual bool uploadData(
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Light> light,
        std::shared_ptr<Material> material,
        glm::mat4& world,
        glm::mat4& view,
        glm::mat4& proj);
    virtual bool updateMeshData(std::shared_ptr<Mesh> mesh, GLuint vbo);
};

class EngineContext;
class Material;
class Mesh;
class Light;
class ProgramManager : public Singleton<ProgramManager> {
public:
    struct ProgramTable {
        const char *technique;
        bool        hasGeometry;
        const char *vertexSrc;
        int         vertexLen;
        const char *fragmentSrc;
        int         fragmentLen;
        const char *gometrySrc;
        int         geometryLen;
    };

    bool preCompile(std::shared_ptr<EngineContext> engineContext);
    std::shared_ptr<Program> getCompatibleProgram(
        std::shared_ptr<Material> material,
        bool hasLight,
        std::shared_ptr<Mesh> mesh);

    friend class Singleton<ProgramManager>;

private:
    ProgramManager();
    virtual ~ProgramManager();

    std::shared_ptr<Program> createProgram(const std::string& name);
    bool isCompatible(bool b1, bool b2);

    std::vector<std::shared_ptr<Program> > mPrograms;
    static ProgramTable builtInProgramTable[];
};

} // namespace dzy

#endif
