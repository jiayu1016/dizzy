#version 300 es

uniform mat4 dzyMVPMatrix;
uniform mat4 dzyMVMatrix;
uniform mat3 dzyNormalMatrix;

in vec3 dzyVertexPosition;
in vec3 dzyVertexNormal;

out vec3 vVertexPositionEyeSpace;
out vec3 vVertexNormalEyeSpace;

void main() {
    gl_Position = dzyMVPMatrix * vec4(dzyVertexPosition, 1.0);
    vVertexPositionEyeSpace = vec3(dzyMVMatrix * vec4(dzyVertexPosition, 1.0));
    vVertexNormalEyeSpace = dzyNormalMatrix * dzyVertexNormal;
}
