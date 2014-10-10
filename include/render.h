#ifndef RENDER_H
#define RENDER_H

#include <memory>
#include <vector>
#include <GLES3/gl3.h>
#include <shader.h>
#include "utils.h"

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

    bool    isValid() const { return mLinked; }
    // bind program
    void    use();
    bool    link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader);
    bool    load(std::shared_ptr<Scene> scene);
    GLint   getAttrib(const char* name) const;
    GLint   getUniform(const char* name) const;

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
    std::vector<GLuint>                         mVBOs;
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
    void drawMesh(std::shared_ptr<Mesh> mesh);

private:
    std::shared_ptr<Program> mProgram;
};

class RenderManager : public Singleton<RenderManager> {
public:
    std::shared_ptr<Render> createDefaultRender() {
        std::shared_ptr<Render> r(new Render);
        mCurrentRender = r;
        return r;
    }

    std::shared_ptr<Render> getCurrentRender() {
        return mCurrentRender;
    }

private:
    std::vector<std::shared_ptr<Render> >   mRenders;
    std::shared_ptr<Render>                 mCurrentRender;
};

} // namespace dzy 

#endif
