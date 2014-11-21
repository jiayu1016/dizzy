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

class Scene;
class Program {
public:
    enum Requirement {
        REQUIRE_VERTEX_COLOR,
        REQUIRE_VERTEX_NORMAL,
        REQUIRE_MATERIAL,
        REQUIRE_LIGHT,
    };

    Program();
    ~Program();

    GLuint getId() const { return mProgramId; }
    bool isValid() const { return mLinked; }
    // bind program
    void use();
    bool link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader);
    bool storeLocation();
    GLint getLocation(const char* name);
    void setRequirement(
        bool requireVertexColor,
        bool requireVertexNormal,
        bool requireMaterial,
        bool requireLight);
    bool hasRequirement(Requirement requirement);

    friend class Shader;

private:
    bool                                        mLinked;
    GLuint                                      mProgramId;
    std::vector<std::shared_ptr<Shader> >       mShaders;
    std::map<std::string, GLint>                mLocations;
    int                                         mRequirement;
};

class EngineContext;
class Material;
class Mesh;
class Light;
class ProgramManager : public Singleton<ProgramManager> {
public:
    bool preCompile(std::shared_ptr<EngineContext> engineContext);
    std::shared_ptr<Program> getCompatibleProgram(
        std::shared_ptr<Material> material,
        bool hasLight,
        std::shared_ptr<Mesh> mesh);

    friend class Singleton<ProgramManager>;

private:
    ProgramManager() {};
    bool isCompatible(bool b1, bool b2);

    std::vector<std::shared_ptr<Program> > mPrograms;
};

} // namespace dzy

#endif
