#include <memory>
#include <functional>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "render.h"

using namespace std;

namespace dzy {

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

bool Shader::compileFromAsset(
    AAssetManager *assetManager, const string &assetFile) {
    assert(assetManager != NULL);

    AAsset* asset = AAssetManager_open(assetManager,
        assetFile.c_str(), AASSET_MODE_BUFFER);

    if (!asset) {
        ALOGE("Failed to open asset: %s", assetFile.c_str());
        return false;
    }

    off_t length = AAsset_getLength(asset);
    unique_ptr<char[]> buffer(new char[length]);
    size_t sz = AAsset_read(asset, buffer.get(), length);
    AAsset_close(asset);
    if (sz != length) {
        ALOGE("Partial read %d bytes", sz);
        return false;
    }

    return compileFromMemory(buffer.get(), sz);
}

Program::Program()
    : mLinked(false)
    , mProgramId(0) {
    ALOGD("Program::Program()");
    mProgramId = glCreateProgram();
    if (!mProgramId) {
        ALOGE("glCreateProgram error");
        return;
    }
}

Program::~Program() {
    ALOGD("Program::~Program()");
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
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexPosition");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex position");
            return false;
        }
        mLocations["dzyVertexPosition"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasVertexColor()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexColor");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex color");
            return false;
        }
        mLocations["dzyVertexColor"] = attribLoc;
    }
    if (scene->atLeastOneMeshHasNormal()) {
        GLint attribLoc = glGetAttribLocation(mProgramId, "dzyVertexNormal");
        if (attribLoc == -1) {
            ALOGE("shader not compatible with scene: vertex normal");
            return false;
        }
        mLocations["dzyVertexNormal"] = attribLoc;
    }

    GLint mvpLoc = glGetUniformLocation(mProgramId, "dzyMVPMatrix");
    if (mvpLoc == -1) {
        ALOGE("shader not compatible with scene: mvp matrix");
        return false;
    }
    mLocations["dzyMVPMatrix"] = mvpLoc;

    GLint mvLoc = glGetUniformLocation(mProgramId, "dzyMVMatrix");
    if (mvLoc != -1)
        mLocations["dzyMVMatrix"] = mvLoc;

    GLint normalMatrixLoc = glGetUniformLocation(mProgramId, "dzyNormalMatrix");
    if (normalMatrixLoc != -1)
        mLocations["dzyNormalMatrix"] = normalMatrixLoc;

    if (scene->getNumMaterials() > 0) {
        GLint loc = -1;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.diffuse");
        if (loc != -1) mLocations["dzyMaterial.diffuse"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.specular");
        if (loc != -1) mLocations["dzyMaterial.specular"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.ambient");
        if (loc != -1) mLocations["dzyMaterial.ambient"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.emission");
        if (loc != -1) mLocations["dzyMaterial.emission"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyMaterial.shininess");
        if (loc != -1) mLocations["dzyMaterial.shininess"] = loc;
    }

    if (scene->getNumLights() > 0) {
        GLint loc = -1;
        loc = glGetUniformLocation(mProgramId, "dzyLight.color");
        if (loc != -1) mLocations["dzyLight.color"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.ambient");
        if (loc != -1) mLocations["dzyLight.ambient"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.position");
        if (loc != -1) mLocations["dzyLight.position"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationConstant");
        if (loc != -1) mLocations["dzyLight.attenuationConstant"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationLinear");
        if (loc != -1) mLocations["dzyLight.attenuationLinear"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.attenuationQuadratic");
        if (loc != -1) mLocations["dzyLight.attenuationQuadratic"] = loc;
        loc = glGetUniformLocation(mProgramId, "dzyLight.strength");
        if (loc != -1) mLocations["dzyLight.strength"] = loc;
    }

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

Render::Render() {
    ALOGD("Render::Render()");
}

Render::~Render() {
    ALOGD("Render::~Render()");
}

bool Render::init(shared_ptr<Scene> scene) {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) {
        ALOGE("EngineContext released while init Render");
        return false;
    }

    shared_ptr<Shader> vtxShader(new Shader(GL_VERTEX_SHADER));
    if (!vtxShader->compileFromAsset(engineContext->getAssetManager(), "vertex.shader")) {
        ALOGE("error compile vertex shader");
        return false;
    }
    shared_ptr<Shader> fragShader(new Shader(GL_FRAGMENT_SHADER));
    if (!fragShader->compileFromAsset(engineContext->getAssetManager(), "fragment.shader")) {
        ALOGE("error compile fragment shader");
        return false;
    }
    shared_ptr<Program> program(new Program);
    if (!program->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return false;
    }
    if (!program->load(scene)) {
        ALOGE("error load data");
        return false;
    }

    // if this routine called twice on the same Render object,
    // the original Program object will be deleted after this assignment,
    // draw routine will get mismatched program object.
    mProgram = program;

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);

    glViewport(0, 0,
        engineContext->getSurfaceWidth(),
        engineContext->getSurfaceHeight());

    return true;
}

bool Render::release() {
    if (mProgram) mProgram.reset();
    return true;
}

bool Render::drawScene(shared_ptr<Scene> scene) {
    shared_ptr<EngineContext> engineContext(getEngineContext());
    if (!engineContext) {
        ALOGE("EngineContext released while rendering a scene");
        return false;
    }

    mProgram->use();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    scene->mCameraModelTransform = glm::mat4(1.0f);

    NodeTree &tree = scene->getNodeTree();
    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        shared_ptr<Node> node(tree.findNode(activeCamera->mName));
        if (node) {
            glm::mat4 transform = glm::mat4(1.0f);
            while(node != tree.mRoot) {
                transform = node->mTransformation * transform;
                node = node->getParent();
            }
            transform = node->mTransformation * transform;
            scene->mCameraModelTransform = transform;
        }

        // Support lighing only when we have a camera
        // Use only one light right now
        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                shared_ptr<Node> node(tree.findNode(light->mName));
                if (node) {
                    glm::mat4 trans = glm::mat4(1.0f);
                    while(node != tree.mRoot) {
                        trans = node->mTransformation * trans;
                        node = node->getParent();
                    }
                    trans = node->mTransformation * trans;

                    glUniform3fv(mProgram->getLocation("dzyLight.color"),
                        1, glm::value_ptr(light->mColorDiffuse));
                    glUniform3fv(mProgram->getLocation("dzyLight.ambient"),
                        1, glm::value_ptr(light->mColorAmbient));
                    glm::mat4 view = activeCamera->getViewMatrix(scene->mCameraModelTransform);
                    glm::vec3 lightPosEyeSpace = glm::vec3(view * trans * glm::vec4(light->mPosition, 1.0f));
                    glUniform3fv(mProgram->getLocation("dzyLight.position"),
                        1, glm::value_ptr(lightPosEyeSpace));
                    glUniform1f(mProgram->getLocation("dzyLight.attenuationConstant"),
                        light->mAttenuationConstant);
                    glUniform1f(mProgram->getLocation("dzyLight.attenuationLinear"),
                        light->mAttenuationLinear);
                    glUniform1f(mProgram->getLocation("dzyLight.attenuationQuadratic"),
                        light->mAttenuationQuadratic);
                    glUniform1f(mProgram->getLocation("dzyLight.strength"), 1.0f);

                }
            }
        }
    }

    using namespace std::placeholders;
    tree.dfsTraversal(scene, bind(&Render::drawNode, this, _1, _2));
    eglSwapBuffers(engineContext->getEGLDisplay(), engineContext->getEGLSurface());

    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    if (!node) return;

    shared_ptr<Node> nd(node);
    NodeTree &tree = scene->getNodeTree();
    glm::mat4 world = glm::mat4(1.0f);
    while(nd != tree.mRoot) {
        world = nd->mTransformation * world;
        nd = nd->getParent();
    }
    world = nd->mTransformation * world;
    glm::mat4 mvp = world;

    //Utils::dump(node->mName.c_str(), node->mTransformation);
    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        //override the aspect read from model
        float surfaceWidth = getEngineContext()->getSurfaceWidth();
        float surfaceHeight = getEngineContext()->getSurfaceHeight();
        activeCamera->setAspect(surfaceWidth/surfaceHeight);
        glm::mat4 view = activeCamera->getViewMatrix(scene->mCameraModelTransform);
        glm::mat4 proj = activeCamera->getProjMatrix();
        glm::mat4 mv = view * world;
        mvp = proj * mv;

        GLint mvLoc = mProgram->getLocation("dzyMVMatrix");
        if (mvLoc != -1) {
            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
        }
        GLint normalMatrixLoc = mProgram->getLocation("dzyNormalMatrix");
        if (normalMatrixLoc != -1) {
            glm::mat3 mvInvTransMatrix = glm::mat3(glm::transpose(glm::inverse(mv)));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvInvTransMatrix));
        }
    }

    glUniformMatrix4fv(mProgram->getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));
    for (size_t i=0; i<node->mMeshes.size(); i++) {
        int meshIdx = node->mMeshes[i];
        drawMesh(scene, meshIdx);
    }
}

void Render::drawMesh(shared_ptr<Scene> scene, int meshIdx) {
    shared_ptr<Mesh> mesh(scene->mMeshes[meshIdx]);
    mProgram->bindBufferObjects(meshIdx);
    //ALOGD("meshIdx: %d, mesh name: %s, material idx: %d",
    //    meshIdx, mesh->mName.c_str(), mesh->mMaterialIndex);
    if (scene->getNumMaterials() > 0) {
        shared_ptr<Material> material(scene->mMaterials[mesh->mMaterialIndex]);
        glm::vec3 diffuse, specular, ambient, emission;
        float shininess;
        material->get(Material::COLOR_DIFFUSE, diffuse);
        material->get(Material::COLOR_SPECULAR, specular);
        material->get(Material::COLOR_AMBIENT, ambient);
        material->get(Material::COLOR_EMISSION, emission);
        material->get(Material::SHININESS, shininess);

        glUniform3fv(mProgram->getLocation("dzyMaterial.diffuse"),
            1, glm::value_ptr(diffuse));
        glUniform3fv(mProgram->getLocation("dzyMaterial.specular"),
            1, glm::value_ptr(specular));
        glUniform3fv(mProgram->getLocation("dzyMaterial.ambient"),
            1, glm::value_ptr(ambient));
        glUniform3fv(mProgram->getLocation("dzyMaterial.emission"),
            1, glm::value_ptr(emission));
        glUniform1f(mProgram->getLocation("dzyMaterial.shininess"), shininess);

    }
    if (mesh->hasVertexPositions()) {
        GLint posLoc = mProgram->getLocation("dzyVertexPosition");
        glEnableVertexAttribArray(posLoc);
        //ALOGD("num vertices: %d, num components: %d, stride: %d",
        //    mesh->getNumVertices(),
        //    mesh->getPositionNumComponent(),
        //    mesh->getPositionBufStride());
        //mesh->dumpVertexPositionBuf();
        glVertexAttribPointer(
            posLoc,
            mesh->getPositionNumComponent(),// size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getPositionBufStride(),   // stride, 0 means tightly packed
            (void*)mesh->getPositionOffset()// offset
        );
    }
    if (mesh->hasVertexColors()) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        GLint colorLoc = mProgram->getLocation("dzyVertexColor");
        glEnableVertexAttribArray(colorLoc);
    }
    if (mesh->hasVertexNormals()) {
        GLint normalLoc = mProgram->getLocation("dzyVertexNormal");
        glEnableVertexAttribArray(normalLoc);
        glVertexAttribPointer(
            normalLoc,
            mesh->getNormalNumComponent(),  // size
            GL_FLOAT,                       // type
            GL_FALSE,                       // normalized
            mesh->getNormalBufStride(),     // stride, 0 means tightly packed
            (void *)mesh->getNormalOffset() // offset
        );
    }

    //ALOGD("num indices: %d, indices buf size: %d",
    //    mesh->getNumIndices(), mesh->getIndexBufSize());
    //mesh->dumpIndexBuf();
    // support only GL_UNSIGNED_INT right now
    glDrawElements(GL_TRIANGLES,            // mode
        mesh->getNumIndices(),              // indices count
        GL_UNSIGNED_INT,                    // type
        (void *)0);                         // offset

    // restore
    if (mesh->hasVertexPositions())
        glDisableVertexAttribArray(mProgram->getLocation("dzyVertexPosition"));

    if (mesh->hasVertexColors()) {
        glDisableVertexAttribArray(mProgram->getLocation("dzyVertexColor"));
    }
    if (mesh->hasVertexNormals()) {
        glDisableVertexAttribArray(mProgram->getLocation("dzyVertexNormal"));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

shared_ptr<EngineContext> Render::getEngineContext() {
    return mEngineContext.lock();
}

const char* Render::glStatusStr() {
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

void Render::setEngineContext(shared_ptr<EngineContext> engineContext) {
    mEngineContext = engineContext;
}

} // namespace dzy
