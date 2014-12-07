#include "transform.h"

namespace dzy {

Transform::Transform(
    const glm::vec3& translation,
    const glm::quat& rotation,
    const glm::vec3& scale)
    : mTranslation(translation)
    , mRotation(rotation)
    , mScale(scale) {
}

Transform::Transform(const glm::vec3& translation, const glm::quat& rotation)
    : Transform(translation, rotation, glm::vec3(1.f, 1.f, 1.f)) {
}

Transform::Transform(const glm::vec3& translation)
    : Transform(translation, glm::quat(1.f, 0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)) {
}

Transform::Transform(const glm::quat& rotation)
    : Transform(glm::vec3(0.f, 0.f, 0.f), rotation, glm::vec3(1.f, 1.f, 1.f)) {
}

Transform::Transform() {
    loadIdentity();
}

Transform::Transform(const Transform& other) {
    mTranslation = other.mTranslation;
    mRotation = other.mRotation;
    mScale = other.mScale;
}

Transform& Transform::operator=(const Transform& rhs) {
    if (this != &rhs) {
        mTranslation = rhs.mTranslation;
        mRotation = rhs.mRotation;
        mScale = rhs.mScale;
    }
    return *this;
}

void Transform::loadIdentity() {
    mTranslation = glm::vec3(0.f, 0.f, 0.f);
    mScale = glm::vec3(1.f, 1.f, 1.f);
    mRotation = glm::quat(1.f, 0.f, 0.f, 0.f);
}

Transform& Transform::setRotation(const glm::quat& rotation) {
    mRotation = rotation;
    return *this;
}

glm::quat Transform::getRotation() {
    return mRotation;
}

Transform& Transform::setTranslation(const glm::vec3& translation) {
    mTranslation = translation;
    return *this;
}

Transform& Transform::setTranslation(float x,float y, float z) {
    mTranslation = glm::vec3(x, y, z);
    return *this;
}

glm::vec3 Transform::getTranslation() {
    return mTranslation;
}

Transform& Transform::setScale(const glm::vec3& scale) {
    mScale = scale;
    return *this;
}

Transform& Transform::setScale(float scale) {
    return setScale(scale, scale, scale);
}

Transform& Transform::setScale(float x, float y, float z) {
    mScale = glm::vec3(x, y, z);
    return *this;
}

glm::vec3 Transform::getScale() {
    return mScale;
}

Transform& Transform::combine(const Transform& parent) {
    mScale *= parent.mScale;
    mRotation = parent.mRotation * mRotation;

    mTranslation *= parent.mScale;
    mTranslation  = parent.mRotation * mTranslation;
    mTranslation += parent.mTranslation;

    return *this;
}

glm::mat4 Transform::toMat4() {
    return glm::translate(glm::mat4(1.f), mTranslation)
        * glm::mat4_cast(mRotation)
        * glm::scale(glm::mat4(1.f), mScale);
}

}
