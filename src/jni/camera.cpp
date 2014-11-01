#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include "log.h"
#include "camera.h"

using namespace std;

namespace dzy {

Camera::Camera()
    : mPosition         (0.f, 0.f, 0.f)
    , mUp               (0.f, 1.f, 0.f)
    , mLookAt           (0.f, 0.f, -1.f)
    , mHorizontalFOV    (0.25f * (float)M_PI)
    , mClipPlaneNear    (0.1f)
    , mClipPlaneFar     (1000.f)
    , mAspect           (0.f) {
}

Camera::Camera(
    glm::vec3           position,
    glm::vec3           up,
    glm::vec3           lookAt,
    float               horizontalFOV,
    float               clipPlaneNear,
    float               clipPlaneFar,
    float               aspect)
    : mPosition         (position)
    , mUp               (up)
    , mLookAt           (lookAt)
    , mHorizontalFOV    (horizontalFOV)
    , mClipPlaneNear    (clipPlaneNear)
    , mClipPlaneFar     (clipPlaneFar)
    , mAspect           (aspect) {
}

void Camera::setAspect(float aspect) {
    mAspect = aspect;
}

#if 0
glm::mat4 Camera::getCameraMatrix() {
    glm::vec3 zaxis = mLookAt;
    zaxis = glm::normalize(zaxis);
    glm::vec3 yaxis = mUp;
    yaxis = glm::normalize(yaxis);
    glm::vec3 xaxis = glm::cross(yaxis, zaxis);

    return glm::lookAt(...);
}
#endif

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(mPosition, mLookAt, mUp);
}

glm::mat4 Camera::getViewMatrix(glm::mat4 modelTransfrom) {
    //Utils::dump("camera model transform", modelTransfrom);
    glm::vec4 newPos = modelTransfrom * glm::vec4(mPosition, 1.0f);
    glm::vec4 newLook = modelTransfrom * glm::vec4(mLookAt, 1.0f);
    //glm::vec4 newUp = modelTransfrom * glm::vec4(mUp, 1.0f);

    glm::vec3 pos = glm::vec3(newPos.x, newPos.y, newPos.z);
    glm::vec3 look = glm::vec3(newLook.x, newLook.y, newLook.z);
    //glm::vec3 up = glm::vec3(newUp.x, newUp.y, newUp.z);

    // up vector doesn't change
    return glm::lookAt(pos, look, mUp);
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
