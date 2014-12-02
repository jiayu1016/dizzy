#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include "log.h"
#include "camera.h"

using namespace std;

namespace dzy {

Camera::Camera(const string& name)
    : Camera(
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(0.f, 1.f, 0.f),
            glm::vec3(0.f, 0.f, -1.f),
            0.25f * (float)M_PI,
            0.1f,
            1000.f,
            1.f,
            name) {
}

Camera::Camera(
    glm::vec3           position,
    glm::vec3           up,
    glm::vec3           lookAt,
    float               horizontalFOV,
    float               clipPlaneNear,
    float               clipPlaneFar,
    float               aspect,
    const string&       name)
    : NameObj           (name)
    , mPosition         (position)
    , mUp               (up)
    , mLookAt           (lookAt)
    , mHorizontalFOV    (horizontalFOV)
    , mClipPlaneNear    (clipPlaneNear)
    , mClipPlaneFar     (clipPlaneFar)
    , mAspect           (aspect) {
}

Camera& Camera::translate(float x, float y, float z) {
    return translate(glm::vec3(x, y, z));
}

Camera& Camera::translate(const glm::vec3& offset) {
    glm::vec3 translate = mLocalTransform.getTranslation();
    mLocalTransform.setTranslation(translate + offset);
    return *this;
}

Camera& Camera::scale(float s) {
    return scale(s, s, s);
}

Camera& Camera::scale(float x, float y, float z) {
    return scale(glm::vec3(x, y, z));
}

Camera& Camera::scale(const glm::vec3& s) {
    glm::vec3 scale = mLocalTransform.getScale();
    mLocalTransform.setScale(s * scale);
    return *this;
}

Camera& Camera::rotate(const glm::quat& rotation) {
    glm::quat rot = mLocalTransform.getRotation();
    mLocalTransform.setRotation(rotation * rot);
    return *this;
}

Camera& Camera::rotate(float axisX, float axisY, float axisZ) {
    glm::quat quaternion(glm::vec3(axisX, axisY, axisZ));
    return rotate(quaternion);
}

void Camera::setAspect(float aspect) {
    mAspect = aspect;
}

// TODO: support (yaw, pitch, roll) euler angle transform
glm::mat4 Camera::getViewMatrix() {
    glm::mat4 localTransform = mLocalTransform.toMat4();
    glm::vec4 newPos = localTransform * glm::vec4(mPosition, 1.0f);
    //glm::vec4 newLook = localTransform * glm::vec4(mLookAt, 1.0f);
    //glm::vec4 newUp = localTransform * glm::vec4(mUp, 1.0f);

    glm::vec3 pos = glm::vec3(newPos.x, newPos.y, newPos.z);
    //glm::vec3 look = glm::vec3(newLook.x, newLook.y, newLook.z);
    //glm::vec3 up = glm::vec3(newUp.x, newUp.y, newUp.z);

    // up vector doesn't change
    return glm::lookAt(pos, mLookAt, mUp);
}

glm::mat4 Camera::getProjMatrix() {
    return glm::perspective(mHorizontalFOV, mAspect, mClipPlaneNear, mClipPlaneFar);
}

void Camera::dump(glm::vec3 pos, glm::vec3 at, glm::vec3 up) {
    PRINT("position: (%+08.6f, %+08.6f, %+08.6f)",
        pos.x, pos.y, pos.z);
    PRINT("at: (%+08.6f, %+08.6f, %+08.6f)",
        at.x, at.y, at.z);
    PRINT("up: (%+08.6f, %+08.6f, %+08.6f)",
        up.x, up.y, up.z);
}

void Camera::dumpParameter() {
    PRINT("*********** Camera Parameters ***********");
    dump(mPosition, mLookAt, mUp);
    PRINT("(fov, aspect, near, far): (%+08.6f, %+08.6f, %+08.6f, %+08.6f)",
        mHorizontalFOV, mAspect, mClipPlaneNear, mClipPlaneFar);
}

} //namespace
