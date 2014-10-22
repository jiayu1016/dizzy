#include <jni.h>
#include <assimp/scene.h>
#include "utils.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
 * does not work, seems not packed
 */
#if 0
void dumpMemLayout(const char *msg, const aiMatrix4x4& mat4) {
    PRINT("********** %s ************", msg);
    for (int i=0; i<16; i+=4) {
        PRINT("%+08.6f %+08.6f %+08.6f %+08.6f",
            *mat4[i], *mat4[i+1], *mat4[i+2], *mat4[i+3]);
    }
}
#endif

void dumpMemLayout(const char *msg, const aiMatrix4x4& mat4) {
    PRINT("********** %s ************", msg);
    PRINT("%+08.6f %+08.6f %+08.6f %+08.6f",
            mat4.a1, mat4.a2, mat4.a3, mat4.a4);
    PRINT("%+08.6f %+08.6f %+08.6f %+08.6f",
            mat4.b1, mat4.b2, mat4.b3, mat4.b4);
    PRINT("%+08.6f %+08.6f %+08.6f %+08.6f",
            mat4.c1, mat4.c2, mat4.c3, mat4.c4);
    PRINT("%+08.6f %+08.6f %+08.6f %+08.6f",
            mat4.d1, mat4.d2, mat4.d3, mat4.d4);
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_wayne_dizzy_test_transform_MainActivity_testMain(JNIEnv* env, jobject obj);
};

JNIEXPORT void JNICALL
Java_com_wayne_dizzy_test_transform_MainActivity_testMain(JNIEnv* env, jobject obj) {
    {
        aiMatrix4x4 identity, result;
        aiVector3D translation(3.f, 4.f, 5.f);
        aiMatrix4x4::Translation(translation, result);
        dumpMemLayout("verify assimp matrix translation component", result);
    }
    {
        glm::mat4 identity;
        glm::vec3 translation(3.f, 4.f, 5.f);
        glm::mat4 result = glm::translate(identity, translation);
        dzy::Utils::dump("verify glm matrix translation component", result);
    }
    {
        glm::mat4 rootTrans(
                1.f, 0.f, 0.f, 0.f,
                0.f, 0.f, -1.f, 0.f,
                0.f, 1.f, 0.f, 0.f,
                0.f, 0.f, 0.f, 1.f);
        glm::mat4 cameraTrans(
                0.707106f, 0.707107f, 0.f, 0.f,
                -0.374709f, 0.374709f, 0.848048f, 0.f,
                0.599661f, -0.599661f, 0.529919f, 0.f,
                7.f, -6.f, 6.f, 1.f);
        glm::vec4 v(3.f, 4.f, 5.f, 1.f);
        glm::vec4 result = rootTrans * v;
        glm::vec3 r(result.x, result.y, result.z);
        dzy::Utils::dump("flip blender z-up to gl y-up", r);

        glm::mat4 view = rootTrans * cameraTrans;
        dzy::Utils::dump("view transform", view);
    }
    {
        glm::mat4 identity(1.f);
        glm::mat4 rootTrans(
                1.f, 0.f, 0.f, 0.f,
                0.f, 0.f, -1.f, 0.f,
                0.f, 1.f, 0.f, 0.f,
                0.f, 0.f, 0.f, 1.f);
        glm::mat4 rotateX = glm::rotate(identity, 58.f * 3.141593f / 180.f, glm::vec3(1.f, 0.f, 0.f));
        glm::mat4 rotateZ = glm::rotate(identity, 45.f * 3.141593f / 180.f, glm::vec3(0.f, 0.f, 1.f));
        glm::mat4 translate = glm::translate(identity, glm::vec3(7.f, -6.f, 6.f));
        glm::mat4 cameraTrans = rootTrans * translate * rotateZ * rotateX;
        dzy::Utils::dump("my view transform", cameraTrans);

        glm::vec4 up(0.f, 1.f, 0.f, 1.f);
        glm::vec4 newUp = cameraTrans * up;
        dzy::Utils::dump("new up vector after transformation", newUp);
    }
}

