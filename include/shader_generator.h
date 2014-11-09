#ifndef SHADER_GENERATOR_H
#define SHADER_GENERATOR_H

#include <string>
#include <sstream>
#include <vector>

#include "program.h"

namespace dzy {

class ShaderVariable {
public:
    ShaderVariable();
    ShaderVariable(const std::string &type, const std::string &name);

    std::string getType() const;
    void setType(const std::string &type);
    std::string getName() const;
    void setName(const std::string &name);

    std::string str() const;

private:
    std::string mType;
    std::string mName;
};

class ShaderStruct {
public:
    ShaderStruct(const std::string& name);
    ShaderStruct& operator<<(const ShaderVariable& shaderVariable);

    void setName(const std::string& name);
    std::string getName() const;
    const std::vector<ShaderVariable>& getMembers() const;
    std::string str() const;
private:
    std::string                 mName;
    std::string                 mPrefix;
    std::vector<ShaderVariable> mMembers;
};

class Material;
class Mesh;
class ShaderGenerator {
public:
    struct Info {
        std::vector<ShaderVariable> mVertexAttribs;
        std::vector<ShaderVariable> mVertexUniforms;
        std::vector<ShaderVariable> mVaryings;
        // vertex output for predefined gl_Position
        ShaderVariable              mVertexOutput;
        std::vector<ShaderVariable> mFragmentUniforms;
        // fragment output, gl_FragColor or gl_Fragdata[n]
        std::vector<ShaderVariable> mFragmentOutputs;
        std::vector<ShaderStruct>   mVertexDeclarations;
        std::vector<ShaderStruct>   mFragmentDeclarations;
        bool                        mHasPosition;
        bool                        mHasNormal;

        Info() : mHasPosition(false), mHasNormal(false) {};
    };

    ShaderGenerator();

    std::shared_ptr<Program> generateProgram(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh);
    std::string buildShader(const Info& info, Shader::ShaderType type);

    void declareStruct(std::ostringstream& os, const Info& info, Shader::ShaderType type);
    void generateUniforms(std::ostringstream& os, const Info& info, Shader::ShaderType type);
    void generateAttributes(std::ostringstream& os, const Info& info);
    void generateVaryings(std::ostringstream& os, const Info& info, Shader::ShaderType type);
    void generateMainStart(std::ostringstream& os, const Info& info, Shader::ShaderType type);
    void generateMainBody(std::ostringstream& os, const Info& info, Shader::ShaderType type);
    void generateMainEnd(std::ostringstream& os, const Info& info, Shader::ShaderType type);
/* 
    void generateNodeMainSection(std::ostringstream os, Info info);
*/
};

}

#endif
