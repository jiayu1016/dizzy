#version 300 es

precision mediump float;

in vec3 vVertexPositionEyeSpace;
in vec3 vVertexNormalEyeSpace;

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 emission;
    float shininess;
};

struct PointLight {
    vec3 color;
    vec3 ambient;
    vec3 position; // in eye space
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    float strength;
};

uniform Material dzyMaterial;
uniform PointLight dzyLight;

// eye direction in eye space is constant
const vec3 EYE_DIRECTION = vec3(0.0, 0.0, 1.0);
// or this ?
//const vec3 EYE_DIRECTION = vec3(0.0, 0.0, 0.0) - vVertexPositionEyeSpace;

out vec4 fragColor;

void main() {
    vec3 lightDirection = dzyLight.position - vVertexPositionEyeSpace;
    float lightDistance = length(lightDirection);
    // normalize lightDirection
    lightDirection = lightDirection / lightDistance;

    float attenuation = 1.0 / (dzyLight.attenuationConstant +
        dzyLight.attenuationLinear * lightDistance +
        dzyLight.attenuationQuadratic * lightDistance * lightDistance);

    vec3 halfVector = normalize(lightDirection + EYE_DIRECTION);

    float diffuse  = max(dot(vVertexNormalEyeSpace, lightDirection), 0.0);
    // Blin-Phong Shading
    float specular = max(dot(vVertexNormalEyeSpace, halfVector), 0.0);

    if (diffuse == 0.0)
        specular = 0.0;
    else
        specular = pow(specular, dzyMaterial.shininess) * dzyLight.strength;

    vec3 scatteredLight = dzyLight.ambient * dzyMaterial.ambient * attenuation
        + dzyLight.color * dzyMaterial.diffuse * diffuse * attenuation;
    vec3 reflectedLight = dzyLight.color * dzyMaterial.specular * specular * attenuation;

    // FIXME: use material diffuse color for the time being
    vec4 objColor = vec4(dzyMaterial.diffuse, 1.0);
    vec3 rgb = min(vec3(1.0),
        dzyMaterial.emission + objColor.rgb * scatteredLight + reflectedLight);

    fragColor = vec4(rgb, objColor.a);
}
