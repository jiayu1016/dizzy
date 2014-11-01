#ifndef LIGHT_H
#define LIGHT_H

#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace dzy {

class Light {
public:
    enum LightSourceType {
        LIGHT_SOURCE_UNDEFINED      = 0x0,
        LIGHT_SOURCE_DIRECTIONAL    = 0x1,
        LIGHT_SOURCE_POINT          = 0x2,
        LIGHT_SOURCE_SPOT           = 0x3,
    };
    Light();
    Light(
        LightSourceType type,
        float           attenuationConstant,
        float           attenuationLinear,
        float           attenuationQuadratic,
        float           angleInnerCone,
        float           angleOuterCone);

    void dumpParameter();

    friend class Render;
    friend class AIAdapter;
private:
    std::string         mName;
    LightSourceType     mType;
    glm::vec3           mPosition;
    glm::vec3           mDirection;
    float               mAttenuationConstant;
    float               mAttenuationLinear;
    float               mAttenuationQuadratic;
    glm::vec3           mColorDiffuse;
    glm::vec3           mColorSpecular;
    glm::vec3           mColorAmbient;
    float               mAngleInnerCone;
    float               mAngleOuterCone;
};

}

#endif
