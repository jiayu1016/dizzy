#include <GLES3/gl3.h>
#include "log.h"
#include "engine_context.h"
#include "scene.h"
#include "program.h"
#include "utils.h"
#include "render.h"

using namespace std;

namespace dzy {

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

    shared_ptr<Node> rootNode = scene->getRootNode();
    if (!rootNode) {
        ALOGW("Scene graph empty, nothing to render");
        return false;
    }

    mProgram->use();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    scene->mCameraModelTransform = glm::mat4(1.0f);

    shared_ptr<Camera> activeCamera(scene->getActiveCamera());
    if (activeCamera) {
        shared_ptr<Node> node(rootNode->findNode(activeCamera->mName));
        if (node) {
            glm::mat4 transform = glm::mat4(1.0f);
            while(node != rootNode) {
                transform = node->mTransformation * transform;
                node = node->getParent();
            }
            transform = node->mTransformation * transform;
            scene->mCameraModelTransform = transform;
        }

        scene->mLightModelTransform = glm::mat4(1.0f);

        // Support lighing only when we have a camera
        // Use only one light right now
        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                shared_ptr<Node> node(rootNode->findNode(light->mName));
                if (node) {
                    glm::mat4 trans = glm::mat4(1.0f);
                    while(node != rootNode) {
                        trans = node->mTransformation * trans;
                        node = node->getParent();
                    }
                    trans = node->mTransformation * trans;
                    scene->mLightModelTransform = trans;
                }
            }
        }
    }

    rootNode->draw(*this, scene);
    eglSwapBuffers(engineContext->getEGLDisplay(), engineContext->getEGLSurface());

    return true;
}

void Render::drawNode(shared_ptr<Scene> scene, shared_ptr<Node> node) {
    if (!node) return;

    shared_ptr<Node> rootNode(scene->getRootNode());
    assert(rootNode);
    glm::mat4 world = glm::mat4(1.0f);
    shared_ptr<Node> nd(node);
    while(nd != rootNode) {
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

        glUniformMatrix4fv(mProgram->getLocation("dzyMVPMatrix"), 1, GL_FALSE, glm::value_ptr(mvp));

        if (scene->getNumLights() > 0) {
            shared_ptr<Light> light(scene->mLights[0]);
            if (light) {
                glUniform3fv(mProgram->getLocation("dzyLight.color"),
                    1, glm::value_ptr(light->mColorDiffuse));
                glUniform3fv(mProgram->getLocation("dzyLight.ambient"),
                    1, glm::value_ptr(light->mColorAmbient));
                glm::vec3 lightPosEyeSpace = glm::vec3(
                    view * scene->mLightModelTransform * glm::vec4(light->mPosition, 1.0f));
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
