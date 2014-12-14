#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "log.h"

namespace dzy {

class Transform {
public:
    Transform(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale);
    Transform(const glm::vec3& translation, const glm::quat& rotation);
    Transform(const glm::vec3& translation);
    Transform(const glm::quat& rotation);
    Transform(const glm::mat4& mat4);
    Transform();
    Transform(const Transform& other);
    Transform& operator=(const Transform& rhs);
    Transform& operator=(const glm::mat4& mat4);
    void loadIdentity();
    friend Transform operator*(Transform& lhs, Transform& rhs);

    Transform& setRotation(const glm::quat& rotation);
    glm::quat getRotation();
    Transform& setTranslation(const glm::vec3& translation);
    Transform& setTranslation(float x,float y, float z);
    glm::vec3 getTranslation();
    Transform& setScale(const glm::vec3& scale);
    Transform& setScale(float scale);
    Transform& setScale(float x, float y, float z);
    glm::vec3 getScale();
    Transform& combine(const Transform& parent);
    glm::mat4 toMat4();
    Transform& fromMat4(const glm::mat4& mat4);
    static void decompose(const glm::mat4& mat4,
        glm::vec3& T, glm::quat& R, glm::vec3& S);
    void dump(Log::Flag f, const char* fmt, ...);

private:
    glm::vec3 mScale;
    glm::quat mRotation;
    glm::vec3 mTranslation;
};

}

#endif
