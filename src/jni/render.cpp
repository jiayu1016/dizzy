#include <memory>
#include <functional>
#include "log.h"
#include "app_context.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

const char* glStatusStr() {
    GLenum error = glGetError();

    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "UNKNOWN_GL_ERROR";
    }
}

/*
static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "in vec4 aPos;\n"
    "in vec4 aColor;\n"
    "uniform mat4 uMVP;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = uMVP * aPos\n"
    "    vColor = aColor;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vColor;\n"
    "}\n";
*/

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "in vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

Shader::Shader(GLenum type)
    : mShaderType(type)
    , mShaderId(0)
    , mInitialized(false) {
}

Shader::~Shader() {
    if (mInitialized && mShaderId)
        glDeleteShader(mShaderId);
}

GLenum Shader::getShaderType() {
    return mShaderType;
}

GLuint Shader::getShaderID() {
    return mShaderId;
}

bool Shader::compileFromMemory(std::vector<uint8_t>& data) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, data);
    if (success) mInitialized = true;
    return success;
}

bool Shader::compileFromMemory(const GLchar *source, const int32_t size) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, source, size);
    if (success) mInitialized = true;
    return success;
}

bool Shader::compileFromFile(const char *strFileName) {
    bool success = ndk_helper::shader::CompileShader(&mShaderId, mShaderType, strFileName);
    if (success) mInitialized = true;
    return success;
}

Program::Program()
    : mLinked(false)
    , mProgramId(0) {
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
    if (mProgramId) glDeleteProgram(mProgramId);
    if (!mVertexBOs.empty())
        glDeleteBuffers(mVertexBOs.size(), &mVertexBOs[0]);
    if (!mIndexBOs.empty())
        glDeleteBuffers(mIndexBOs.size(), &mIndexBOs[0]);
}

void Program::use() {
    glUseProgram(mProgramId);
}

void Program::bindBufferObjects(int meshIdx) {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBOs[meshIdx]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBOs[meshIdx]);
}

bool Program::link(std::shared_ptr<Shader> vtxShader, std::shared_ptr<Shader> fragShader) {
    if (isValid()) {
        ALOGW("Program already linked");
        return false;
    }

    // vertex always slot 0, fragment always slot 1
    mShaders.push_back(vtxShader);
    mShaders.push_back(fragShader);

    // in order to support other shaders in future
    for (auto iter = mShaders.begin(); iter != mShaders.end(); iter++) {
        if (*iter) glAttachShader(mProgramId, (*iter)->getShaderID());
    }

    if (!ndk_helper::shader::LinkProgram(mProgramId)) {
        ALOGE("error link program");
        return false;
    }

    if (!ndk_helper::shader::ValidateProgram(mProgramId)) {
        ALOGE("program validation fail");
        return false;
    }

    mLinked = true;

    return true;
}

bool Program::load(std::shared_ptr<Scene> scene) {
    if (scene->atLeastOneMeshHasVertexPosition()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "aPos");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex position");
            return false;
        }
        mLocations["aPos"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasVertexColor()) {
        ALOGD("at lease one mesh has vertex color");
        GLint attribLoc = glGetAttribLocation(mProgramId, "aColor");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex color");
            return false;
        }
        mLocations["aColor"] = attribLoc;
    }

    /*
    GLint mvpLoc = glGetUniformLocation(mProgramId, "uMVP");
    if (mvpLoc == -1) {
        ALOGE("shader not compatible with scene: mvp matrix");
        return false;
    }
    mLocations["aColor"] = mvpLoc;
    */

    // TODO: check other attributes

    size_t numMeshes = scene->getNumMeshes();
    mVertexBOs.resize(numMeshes);
    mIndexBOs.resize(numMeshes);
    for(size_t i = 0; i < numMeshes; ++i) {
        // Load vertex and index data into buffer object
        shared_ptr<Mesh> mesh(scene->mMeshes[i]);
        glGenBuffers(1, &mVertexBOs[i]);
        glGenBuffers(1, &mIndexBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, mesh->getVertexBufSize(),
            mesh->getVertexBuf(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexBufSize(),
            mesh->getIndexBuf(), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

GLint Program::getLocation(const char* name) {
    if (!isValid()) {
        ALOGE("Program not valid");
        return -1;
    }
    if (mLocations.find(name) == mLocations.end()) {
        ALOGE("%s not found in shader", name);
        return -1;
    }
    return mLocations[name];
}

bool Render::init(shared_ptr<Scene> scene) {
    shared_ptr<Shader> vtxShader(new Shader(GL_VERTEX_SHADER));
    if (!vtxShader->compileFromMemory(VERTEX_SHADER,
            sizeof(VERTEX_SHADER))) {
        ALOGE("error compile vertex shader");
        return false;
    }
    shared_ptr<Shader> fragShader(new Shader(GL_FRAGMENT_SHADER));
    if (!fragShader->compileFromMemory(FRAGMENT_SHADER,
        sizeof(FRAGMENT_SHADER))) {
        ALOGE("error compile fragment shader");
        return false;
    }
    shared_ptr<Program> program(new Program());
    if (!program->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return false;
    }
    if (!program->load(scene)) {
        ALOGE("error load data");
        return false;
    }
    mProgram = program;

    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);

    shared_ptr<AppContext> appContext(getAppContext());
    if (appContext)
        glViewport(0, 0,
            appContext->getSurfaceWidth(),
            appContext->getSurfaceHeight());
    else {
        ALOGE("AppContext released while render trying to init");
        return false;
    }

    return true;
}

bool Render::release() {
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    NodeTree &tree = scene->getNodeTree();
    using namespace std::placeholders;
    mProgram->use();
    tree.dfsTraversal(scene, bind(&Render::drawNode, this, _1, _2));
    shared_ptr<AppContext> appContext(getAppContext());
    if (appContext)
        eglSwapBuffers(appContext->getEGLDisplay(), appContext->getEGLSurface());
    else {
        ALOGE("AppContext released while render trying to draw scene");
        return false;
    }
    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    //ALOGD("Node %s has %d meshes", node->mName.c_str(), node->mMeshes.size());
    for (size_t i=0; i<node->mMeshes.size(); i++) {
        int meshIdx = node->mMeshes[i];
        shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
        drawMesh(mesh, i);
    }
}

void Render::drawMesh(shared_ptr<Mesh> mesh, int meshIdx) {
    ALOGD("meshIdx: %d, mesh name: %s", meshIdx, mesh->mName.c_str());
    glClear(GL_COLOR_BUFFER_BIT);

    mProgram->bindBufferObjects(meshIdx);
    if (mesh->hasPositions()) {
        GLint posLoc = mProgram->getLocation("aPos");
        glEnableVertexAttribArray(posLoc);
        ALOGD("num vertices: %d, num components: %d, stride: %d",
            mesh->getNumVertices(),
            mesh->getVertexNumComponent(),
            mesh->getVertexBufStride());
        mesh->dumpVertexBuf();
        glVertexAttribPointer(
            posLoc,
            mesh->getVertexNumComponent(),  // size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            0,                              // stride, 0 means tightly packed
            (void*)0                        // offset
        );
    }
    if (mesh->hasVertexColors()) {
        glEnableVertexAttribArray(mProgram->getLocation("aColor"));
    }
    // TODO: check other attributes in mesh

    ALOGD("num indices: %d, indices buf size: %d",
        mesh->getNumIndices(), mesh->getIndexBufSize());
    mesh->dumpIndexBuf();
    // support only GL_UNSIGNED_INT right now
    glDrawElements(GL_TRIANGLES,            // mode
        mesh->getNumIndices(),              // indices count
        GL_UNSIGNED_INT,                    // type
        (void *)0);                         // offset

    // restore
    if (mesh->hasPositions())
        glDisableVertexAttribArray(mProgram->getLocation("aPos"));

    if (mesh->hasVertexColors()) {
        glDisableVertexAttribArray(mProgram->getLocation("aColor"));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

shared_ptr<AppContext> Render::getAppContext() {
    return mAppContext.lock();
}

void Render::setAppContext(shared_ptr<AppContext> appContext) {
    mAppContext = appContext;
}


} // namespace dzy
