#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include "log.h"
#include "camera.h"
#include "utils.h"

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

Camera& Camera::pitch(float angle) {
    return rotate(angle, 0, 0);
}

Camera& Camera::yaw(float angle) {
    return rotate(0, angle, 0);
}

Camera& Camera::roll(float angle) {
    return rotate(0, 0, angle);
}

void Camera::resetTransform() {
    mLocalTransform.loadIdentity();
}

void Camera::setAspect(float aspect) {
    mAspect = aspect;
}

float Camera::getAspect() {
    return mAspect;
}
void Camera::setHorizontalFOV(float fov) {
    mHorizontalFOV = fov;
}

float Camera::getHorizontalFOV() {
    return mHorizontalFOV;
}

void Camera::setNearPlane(float near) {
    mClipPlaneNear = near;
}

float Camera::getNearPlane() {
    return mClipPlaneNear;
}

void Camera::setFarPlane(float far) {
    mClipPlaneFar = far;
}

float Camera::getFarPlane() {
    return mClipPlaneFar;
}

void Camera::setPostion(const glm::vec3& pos) {
    mPosition = pos;
}

glm::vec3 Camera::getPosition() {
    return mPosition;
}

void Camera::setUp(const glm::vec3& up) {
    mUp = glm::normalize(up);
}

glm::vec3 Camera::getUp() {
    return mUp;
}

void Camera::setLookAt(const glm::vec3& at) {
    mLookAt = at;
}

glm::vec3 Camera::getLookAt() {
    return mLookAt;
}

glm::mat4 Camera::getViewMatrix() {
    glm::mat4 localTransform = mLocalTransform.toMat4();

    glm::vec4 newPos = localTransform * glm::vec4(mPosition, 1.0f);
    glm::vec4 newLook = localTransform * glm::vec4(mLookAt, 1.0f);
    glm::vec4 newUp = localTransform * glm::vec4(mUp, 1.0f);

    glm::vec3 pos = glm::vec3(newPos.x, newPos.y, newPos.z);
    glm::vec3 look = glm::vec3(newLook.x, newLook.y, newLook.z);
    glm::vec3 up = glm::vec3(newUp.x, newUp.y, newUp.z);

    return glm::lookAt(pos, look, up);
}

glm::mat4 Camera::getProjMatrix() {
    return glm::perspective(mHorizontalFOV, mAspect, mClipPlaneNear, mClipPlaneFar);
}

} //namespace
