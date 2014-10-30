#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <vector>
#include <memory>
#include <GLES3/gl3.h>

class AAssetManager;

namespace dzy {

class Shader {
public:
    Shader(GLenum type);
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
    enum {
        MAX_SHADERS = 2,
    };
    Program();
    ~Program();

    GLuint  getId() const { return mProgramId; }
    bool    isValid() const { return mLinked; }
    // bind program
    void    use();
    void    bindBufferObjects(int meshIdx);
    bool    link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader);
    bool    load(std::shared_ptr<Scene> scene);
    GLint   getLocation(const char* name);

    friend class Shader;
private:
    bool                                        mLinked;
    GLuint                                      mProgramId;
    std::vector<std::shared_ptr<Shader> >       mShaders;
    std::map<std::string, GLint>                mLocations;
    GLint                                       mProjectionMatrixLoc;
    GLint                                       mColorMatrixLoc;
    GLint                                       mColorLoc;
    // the order must be consistent with the mesh vector in scene
    std::vector<GLuint>                         mVertexBOs;
    std::vector<GLuint>                         mIndexBOs;
};

class EngineContext;
class ProgramManager : public Singleton<ProgramManager> {
public:
    bool preCompile(std::shared_ptr<EngineContext> engineContext);
    std::shared_ptr<Program> getDefaultProgram();

    friend class Singleton<ProgramManager>;

private:
    ProgramManager() {};

    std::vector<std::shared_ptr<Program> > mPrograms;
};

} // namespace dzy

#endif
