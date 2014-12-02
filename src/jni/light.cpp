#include "log.h"
#include "light.h"

using namespace std;

namespace dzy {

Light::Light()
    : mType                 (LIGHT_SOURCE_UNDEFINED)
    , mAttenuationConstant  (0.f)
    , mAttenuationLinear    (1.f)
    , mAttenuationQuadratic (0.f)
    , mAngleInnerCone       ((float)M_PI)
    , mAngleOuterCone       ((float)M_PI) {
}

Light::Light(
    LightSourceType type,
    float attenuationConstant,
    float attenuationLinear,
    float attenuationQuadratic,
    float angleInnerCone,
    float angleOuterCone)
    : mType                 (type)
    , mAttenuationConstant  (attenuationConstant)
    , mAttenuationLinear    (attenuationLinear)
    , mAttenuationQuadratic (attenuationQuadratic)
    , mAngleInnerCone       (angleInnerCone)
    , mAngleOuterCone       (angleOuterCone) {
} 

glm::vec3 Light::getPosition() {
    return mPosition;
}

glm::vec3 Light::getDirection() {
    return mDirection;
}

float Light::getAttenuationConstant() {
    return mAttenuationConstant;
}

float Light::getAttenuationLinear() {
    return mAttenuationLinear;
}

float Light::getAttenuationQuadratic() {
    return mAttenuationQuadratic;
}

glm::vec3 Light::getColorDiffuse() {
    return mColorDiffuse;
}

glm::vec3 Light::getColorSpecular() {
    return mColorSpecular;
}

glm::vec3 Light::getColorAmbient() {
    return mColorAmbient;
}

float Light::getAngleInnerCone() {
    return mAngleInnerCone;
}

float Light::getAngleOuterCone() {
    return mAngleOuterCone;
}

Light& Light::translate(float x, float y, float z) {
    return translate(glm::vec3(x, y, z));
}

Light& Light::translate(const glm::vec3& offset) {
    glm::vec3 translate = mLocalTransform.getTranslation();
    mLocalTransform.setTranslation(translate + offset);
    return *this;
}

Light& Light::scale(float s) {
    return scale(s, s, s);
}

Light& Light::scale(float x, float y, float z) {
    return scale(glm::vec3(x, y, z));
}

Light& Light::scale(const glm::vec3& s) {
    glm::vec3 scale = mLocalTransform.getScale();
    mLocalTransform.setScale(s * scale);
    return *this;
}

Light& Light::rotate(const glm::quat& rotation) {
    glm::quat rot = mLocalTransform.getRotation();
    mLocalTransform.setRotation(rotation * rot);
    return *this;
}

Light& Light::rotate(float axisX, float axisY, float axisZ) {
    glm::quat quaternion(glm::vec3(axisX, axisY, axisZ));
    return rotate(quaternion);
}

glm::mat4 Light::getTransform() {
    return mLocalTransform.toMat4();
}

void Light::dumpParameter() {
    ALOGD("light: %s\n"
        "position: (%f, %f, %f), direction: (%f, %f, %f)\n"
        "diffuse: (%f, %f, %f), specular: (%f, %f, %f), ambient: (%f, %f, %f)\n"
        "mAngleInnerCone: %f , mAngleOuterCone: %f\n"
        "mAttenuationConstant: %f, mAttenuationLinear: %f, mAttenuationQuadratic: %f",
        getName().c_str(),
        mPosition.x, mPosition.y, mPosition.z,
        mDirection.x, mDirection.y, mDirection.z,
        mColorDiffuse.x, mColorDiffuse.y, mColorDiffuse.z,
        mColorSpecular.x, mColorSpecular.y, mColorSpecular.z,
        mColorAmbient.x, mColorAmbient.y, mColorAmbient.z,
        mAngleInnerCone, mAngleOuterCone,
        mAttenuationConstant, mAttenuationLinear, mAttenuationQuadratic);
}

}
