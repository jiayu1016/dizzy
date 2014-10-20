#ifndef RENDER_H
#define RENDER_H

#include <memory>
#include <vector>
#include <GLES3/gl3.h>
#include <shader.h>
#include "utils.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace dzy {

extern const char* glStatusStr();
class AppContext;

class Shader {
public:
    Shader(GLenum type);
    ~Shader();
    GLenum getShaderType();
    GLuint getShaderID();

    bool compileFromMemory(std::vector<uint8_t>& data);
    bool compileFromMemory(const GLchar *source, const int32_t size);
    bool compileFromFile(const char *strFileName);

private:
    bool        mInitialized;
    GLuint      mShaderId;
    GLenum      mShaderType;
};

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

class Scene;
class Node;
class Mesh;
class Render {
public:
    bool init(std::shared_ptr<Scene> scene);
    bool release();
    bool drawScene(std::shared_ptr<Scene> scene);
    void drawNode(std::shared_ptr<Scene> scene, std::shared_ptr<Node> node);    
    void drawMesh(std::shared_ptr<Scene> scene, int meshIdx);
    std::shared_ptr<AppContext> getAppContext();
    static const char* glStatusStr();

    friend class AppContext;
private:
    void setAppContext(std::shared_ptr<AppContext> appContext);

    std::shared_ptr<Program>    mProgram;
    std::weak_ptr<AppContext>   mAppContext;

};

} // namespace dzy 

#endif
