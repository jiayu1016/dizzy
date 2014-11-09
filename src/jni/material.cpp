#include "material.h"

using namespace std;

namespace dzy {

Material::Material(const string& name)
    : mName(name)
    , mDiffuse(0.f, 0.f, 0.f)
    , mSpecular(0.f, 0.f, 0.f)
    , mAmbient(1.f, 1.f, 1.f)
    , mEmission(0.f, 0.f, 0.f)
    , mShininess(0.f)
    , mFlags(0) {
}

Material::Material(const Material& rhs) {
    operator=(rhs);
}

Material& Material::operator =(const Material& rhs) {
    if (this != &rhs) {
        mName       = rhs.mName;
        mDiffuse    = rhs.mDiffuse;
        mSpecular   = rhs.mSpecular;
        mAmbient    = rhs.mAmbient;
        mEmission   = rhs.mEmission;
        mShininess  = rhs.mShininess;
        mFlags      = rhs.mFlags;
    }
    return *this;
}

Material::~Material() {
}

shared_ptr<Material> Material::getDefault() {
    return shared_ptr<Material>(new Material("default"));
}

bool Material::hasAmbient() const {
    return (mFlags & F_AMBIENT) != 0;
}

glm::vec3 Material::getAmbient() const {
    return mAmbient;
}

void Material::setAmbient(const glm::vec3& color) {
    mFlags |= F_AMBIENT;
    mAmbient = color;
}

void Material::clearAmbient() {
    mFlags &= ~F_AMBIENT;
    mAmbient = glm::vec3(1.f, 1.f, 1.f);
}

bool Material::hasDiffuse() const {
    return (mFlags & F_DIFFUSE) != 0;
}

glm::vec3 Material::getDiffuse() const {
    return mDiffuse;
}

void Material::setDiffuse(const glm::vec3& color) {
    mFlags |= F_DIFFUSE;
    mDiffuse = color;
}

void Material::clearDiffuse() {
    mFlags &= ~F_DIFFUSE;
    mDiffuse = glm::vec3(0.f, 0.f, 0.f);
}

bool Material::hasSpecular() const {
    return (mFlags & F_SPECULAR) != 0;
}

glm::vec3 Material::getSpecular() const {
    return mSpecular;
}

void Material::setSpecular(const glm::vec3& color) {
    mFlags |= F_SPECULAR;
    mSpecular = color;
}

void Material::clearSpecular() {
    mFlags &= ~F_SPECULAR;
    mSpecular= glm::vec3(0.f, 0.f, 0.f);
}

bool Material::hasEmission() const {
    return (mFlags & F_EMISSION) != 0;
}

glm::vec3 Material::getEmission() const {
    return mEmission;
}

void Material::setEmission(const glm::vec3& color) {
    mFlags |= F_EMISSION;
    mEmission = color;
}

void Material::clearEmission() {
    mFlags &= ~F_EMISSION;
    mEmission = glm::vec3(0.f, 0.f, 0.f);
}

float Material::getShininess() const {
    return mShininess;
}

void Material::setShininess(float shininess) {
    mShininess = shininess;
}

string Material::getName() {
    return mName;
}

void Material::setName(const string& name) {
    mName = name;
}

} //namespace
