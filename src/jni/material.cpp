#include "material.h"

using namespace std;

namespace dzy {

bool Material::get(MaterialType type, glm::vec3& color) {
    switch(type) {
    case COLOR_DIFFUSE:
        color = mDiffuse;
        break;
    case COLOR_SPECULAR:
        color = mSpecular;
        break;
    case COLOR_AMBIENT:
        color = mAmbient;
        break;
    case COLOR_EMISSION:
        color = mEmission;
        break;
    default:
        return false;
    }
    return true;
}

bool Material::get(MaterialType type, float& value) {
    switch(type) {
    case SHININESS:
        value = mShininess;
        break;
    default:
        return false;
    }
    return true;
}

} //namespace
