#version 450

layout ( location = 0 ) in vec2 vTexCoord;

layout (location = 0) out vec4 outColor;

layout( set = 1, binding = 0 ) uniform sampler2D uSampler;

layout( set = 2, binding = 0 ) uniform sampler2D uSampNorm;

layout( set = 3, binding = 0 ) uniform sampler2D uSampRough;

layout( set = 4, binding = 0 ) uniform sampler2D uSampEm;

layout( set = 5, binding = 0 ) uniform sampler2D uSampDs;

void main() {
    outColor = vec4(texture(uSampNorm,vTexCoord).rgb,1.0f);
}