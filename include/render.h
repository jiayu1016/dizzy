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
    Shader(GLenum type)
        : mShaderType(type)
        , mShaderId(0) { }

    GLenum getShaderType() {return mShaderType;}
    GLuint getShaderID() {return mShaderId;}

    bool compileFromMemory(std::vector<uint8_t>& data) {
        return ndk_helper::shader::CompileShader(&mShaderId, mShaderType, data);
    }

    bool compileFromMemory(const GLchar *source, const int32_t size) {
        return ndk_helper::shader::CompileShader(&mShaderId, mShaderType, source, size);
    }

    bool compileFromFile(const char *strFileName) {
        return ndk_helper::shader::CompileShader(&mShaderId, mShaderType, strFileName);
    }

    bool linkProgram(const GLuint prog) {
        return true;
    }

private:
    GLuint mShaderId;
    GLenum mShaderType;
};

class Program {
public:
    Program() { };
    ~Program() { };

    bool isValid() const { return true; }

    // bind program
    void use() { };

    GLuint getAttrib(const char* name) const;
    GLint getUniform(const char* name) const;

private:
    GLuint mProgram;
    GLuint mVertexShader;
    GLuint mFragmentShader;
    GLint mProjectionMatrixLoc;
    GLint mColorMatrixLoc;
    GLint mTextureMatrixLoc;
    GLint mSamplerLoc;
    GLint mColorLoc;
};

class Scene;
class Node;
class Render {
public:
    bool init();
    bool release();
    bool drawScene(std::shared_ptr<Scene> scene);
    void drawNode(std::shared_ptr<Scene> scene, std::shared_ptr<Node> node);
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
