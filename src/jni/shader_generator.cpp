#include <sstream>
#include <algorithm>
#include "material.h"
#include "mesh.h"
#include "shader_generator.h"

using namespace std;

namespace dzy {

ShaderVariable::ShaderVariable() {
}

ShaderVariable::ShaderVariable(const std::string &type, const std::string &name)
    : NameObj(name)
    , mType(type) {
}

string ShaderVariable::getType() const {
    return mType;
}

void ShaderVariable::setType(const string &type) {
    mType = type;
}

string ShaderVariable::str() const {
    return mType + " " + getName() + ";\n";
}

ShaderStruct::ShaderStruct(const string& name)
    : NameObj(name) {
}

ShaderStruct& ShaderStruct::operator<<(const ShaderVariable& shaderVariable) {
    mMembers.push_back(shaderVariable);
    return *this;
}

const vector<ShaderVariable>& ShaderStruct::getMembers() const {
    return mMembers;
}

string ShaderStruct::str() const {
    ostringstream oss;
    oss << "struct " << getName() << " {\n";
    for_each(mMembers.begin(), mMembers.end(), [&oss](ShaderVariable shaderVariable){
        oss << "\t" << shaderVariable.str() << "\n";
    });
    oss << "};\n";
    return oss.str();
}

ShaderGenerator::ShaderGenerator() {
}

shared_ptr<Program> ShaderGenerator::generateProgram(
    shared_ptr<Material> material, shared_ptr<Mesh> mesh) {
    Info info;
    if (mesh->hasVertexPositions()) {
        info.mVertexAttribs.push_back(ShaderVariable("vec3", "dzyVertexPosition"));
        info.mVertexUniforms.push_back(ShaderVariable("mat4", "dzyMVPMatrix"));
        if (mesh->hasVertexNormals()) {
            info.mVertexAttribs.push_back(ShaderVariable("vec3", "dzyVertexNormal"));
            info.mVertexUniforms.push_back(ShaderVariable("mat4", "dzyMVMatrix"));
            info.mVertexUniforms.push_back(ShaderVariable("mat3", "dzyNormalMatrix"));
            info.mVaryings.push_back(ShaderVariable("vec3", "vVertexPositionEyeSpace"));
            info.mVaryings.push_back(ShaderVariable("vec3", "vVertexNormalEyeSpace"));
            info.mHasNormal = true;

            ShaderStruct lightStruct("PointLight");
            lightStruct << ShaderVariable("vec3", "color");
            lightStruct << ShaderVariable("vec3", "ambient");
            lightStruct << ShaderVariable("vec3", "position");
            lightStruct << ShaderVariable("float", "attenuationConstant");
            lightStruct << ShaderVariable("float", "attenuationLinear");
            lightStruct << ShaderVariable("float", "attenuationQuadratic");
            lightStruct << ShaderVariable("float", "strength");
            info.mFragmentDeclarations.push_back(lightStruct);
            info.mFragmentUniforms.push_back(ShaderVariable("PointLight", "dzyLight"));
        }
        info.mHasPosition = true;
    }
    ShaderStruct materialStruct("Material");
    if (material->hasDiffuse())
        materialStruct << ShaderVariable("vec3", "diffuse");
    if (material->hasSpecular())
        materialStruct << ShaderVariable("vec3", "specular");
    if (material->hasAmbient())
        materialStruct << ShaderVariable("vec3", "ambient");
    if (material->hasEmission())
        materialStruct << ShaderVariable("vec3", "emission");
    materialStruct << ShaderVariable("float", "shininess");
    info.mFragmentDeclarations.push_back(materialStruct);
    info.mFragmentUniforms.push_back(ShaderVariable("Material", "dzyMaterial"));

    info.mVertexOutput = ShaderVariable("vec4", "gl_Position");
    info.mFragmentOutputs.push_back(ShaderVariable("vec4", "fragColor"));

    string vtxSource    = buildShader(info, Shader::Vertex);
    string fragSource   = buildShader(info, Shader::Fragment);

    shared_ptr<Shader> vtxShader(new Shader(Shader::Vertex));
    if (!vtxShader->compileFromMemory(vtxSource.c_str(), vtxSource.size())) {
        ALOGE("error compile vertex shader");
        return nullptr;
    }
    shared_ptr<Shader> fragShader(new Shader(Shader::Fragment));
    if (!fragShader->compileFromMemory(fragSource.c_str(), fragSource.size())) {
        ALOGE("error compile fragment shader");
        return nullptr;
    }
    shared_ptr<Program> program(new Program);
    if (!program->link(vtxShader, fragShader)) {
        ALOGE("error link program");
        return nullptr;
    }

    program->use();
    program->storeLocation();

    return program;
}

string ShaderGenerator::buildShader(const Info& info, Shader::ShaderType type) {
    ostringstream oss;
    oss << "#version 300 es\n";

    declareStruct(oss, info, type);

    if (type == Shader::Vertex) {
        generateAttributes(oss, info);
    }
    generateVaryings(oss, info, type);

    generateUniforms(oss, info, type);

    generateMainStart(oss, info, type);

    generateMainBody(oss, info, type);

    generateMainEnd(oss, info, type);

    return oss.str();
}

void ShaderGenerator::declareStruct(
    ostringstream& os, const Info& info, Shader::ShaderType type) {
    if (type == Shader::Vertex) {
        for (auto it = info.mVertexDeclarations.begin();
            it != info.mVertexDeclarations.end(); it++) {
            os << (*it).str();
        }
    } else if (type == Shader::Fragment) {
        for (auto it = info.mFragmentDeclarations.begin();
            it != info.mFragmentDeclarations.end(); it++) {
            os << (*it).str();
        }
    }
}

void ShaderGenerator::generateUniforms(
    std::ostringstream& os, const Info& info, Shader::ShaderType type) {
    if (type == Shader::Vertex && !info.mVertexUniforms.empty()) {
        for (auto it = info.mVertexUniforms.begin();
            it != info.mVertexUniforms.end(); it++) {
            ShaderVariable shaderVariable = *it;
            string type = shaderVariable.getType();
            string name = shaderVariable.getName();
            os << "uniform " << type << " " << name << ";\n";
        }
    } else if (type == Shader::Fragment && !info.mFragmentUniforms.empty()) {
        for (auto it = info.mFragmentUniforms.begin();
            it != info.mFragmentUniforms.end(); it++) {
            ShaderVariable shaderVariable = *it;
            string type = shaderVariable.getType();
            string name = shaderVariable.getName();
            os << "uniform " << type << " " << name << ";\n";
        }
    }
}

void ShaderGenerator::generateAttributes(ostringstream& os, const Info& info) {
    for (auto it = info.mVertexAttribs.begin(); it != info.mVertexAttribs.end(); it++) {
        ShaderVariable shaderVariable = *it;
        string type = shaderVariable.getType();
        string name = shaderVariable.getName();
        os << "in " << type << " " << name << ";\n";
    }
}

void ShaderGenerator::generateVaryings(
    ostringstream& os, const Info& info, Shader::ShaderType type) {
    string prefix;
    prefix = (type == Shader::Vertex) ? "out " : "in ";
    for (auto it = info.mVaryings.begin(); it != info.mVaryings.end(); it++) {
        ShaderVariable shaderVariable = *it;
        string type = shaderVariable.getType();
        string name = shaderVariable.getName();
        os << prefix << type << " " << name << ";\n";
    }
}

void ShaderGenerator::generateMainStart(
    ostringstream& os, const Info& info, Shader::ShaderType type) {
    if (type == Shader::Fragment) {
        for (auto it = info.mFragmentOutputs.begin();
            it != info.mFragmentOutputs.end(); it++) {
            os << "out " << (*it).str();
        }
    }
    os << "void main() {\n";
}

void ShaderGenerator::generateMainBody(
    ostringstream& os, const Info& info, Shader::ShaderType type) {
    if (type == Shader::Vertex) {
        if (info.mHasPosition)
            os << "\t" << info.mVertexOutput.getName() << " = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);\n";
        if (info.mHasNormal) {
            os << "\t" << "vVertexPositionEyeSpace = vec3(dzyMVMatrix * vec4(dzyVertexPosition, 1.0));\n";
            os << "\t" << "vVertexNormalEyeSpace = dzyNormalMatrix * dzyVertexNormal;\n";
        }
    } else if (type == Shader::Fragment) {
        if (info.mHasNormal) {
            os << "\tconst vec3 EYE_DIRECTION = vec3(0.0, 0.0, 1.0);\n";
            os << "\tvec3 lightDirection = dzyLight.position - vVertexPositionEyeSpace;\n";
            os << "\tfloat lightDistance = length(lightDirection);\n";
            os << "\tlightDirection = lightDirection / lightDistance;\n";
            os << "\tfloat attenuation = 1.0 / (dzyLight.attenuationConstant +"
                  "dzyLight.attenuationLinear * lightDistance +"
                  "dzyLight.attenuationQuadratic * lightDistance * lightDistance);\n";
            os << "\tvec3 halfVector = normalize(lightDirection + EYE_DIRECTION);\n";
            os << "\tfloat diffuse  = max(dot(vVertexNormalEyeSpace, lightDirection), 0.0);\n";
            os << "\t// Blin-Phong Shading\n";
            os << "\tfloat specular = max(dot(vVertexNormalEyeSpace, halfVector), 0.0);\n";
            os << "\tif (diffuse == 0.0) specular = 0.0;\n";
            os << "\telse specular = pow(specular, dzyMaterial.shininess) * dzyLight.strength;\n";
            os << "\tvec3 scatteredLight = dzyLight.ambient * dzyMaterial.ambient * attenuation"
                  " + dzyLight.color * dzyMaterial.diffuse * diffuse * attenuation;\n";
            os << "\tvec3 reflectedLight = dzyLight.color * dzyMaterial.specular * specular * attenuation;\n";
            os << "\t// FIXME: use material diffuse color for the time being\n";
            os << "\tvec4 objColor = vec4(dzyMaterial.diffuse, 1.0);\n";
            os << "\tvec3 rgb = min(vec3(1.0),"
                  " dzyMaterial.emission + objColor.rgb * scatteredLight + reflectedLight);\n";
            os << "\tfragColor = vec4(rgb, objColor.a);\n";
        } else {
            os << "\tfragColor = vec4(dzyMaterial.diffuse + dzyMaterial.ambient + dzyMaterial.specular, 1.0);\n";
        }
    }
}

void ShaderGenerator::generateMainEnd(
    ostringstream& os, const Info& info, Shader::ShaderType type) {
    os << "}\n";
}

}
