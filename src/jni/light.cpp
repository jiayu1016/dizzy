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

glm::mat4 Light::getTransform() {
    return mTransform;
}

void Light::setTransform(const glm::mat4& trans) {
    mTransform = trans;
}

void Light::dumpParameter() {
    ALOGD("light: %s\n"
        "position: (%f, %f, %f), direction: (%f, %f, %f)\n"
        "diffuse: (%f, %f, %f), specular: (%f, %f, %f), ambient: (%f, %f, %f)\n"
        "mAngleInnerCone: %f , mAngleOuterCone: %f\n"
        "mAttenuationConstant: %f, mAttenuationLinear: %f, mAttenuationQuadratic: %f",
        mName.c_str(),
        mPosition.x, mPosition.y, mPosition.z,
        mDirection.x, mDirection.y, mDirection.z,
        mColorDiffuse.x, mColorDiffuse.y, mColorDiffuse.z,
        mColorSpecular.x, mColorSpecular.y, mColorSpecular.z,
        mColorAmbient.x, mColorAmbient.y, mColorAmbient.z,
        mAngleInnerCone, mAngleOuterCone,
        mAttenuationConstant, mAttenuationLinear, mAttenuationQuadratic);
}

}
