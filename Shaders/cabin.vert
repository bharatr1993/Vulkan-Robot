#version 450

layout( set = 0, binding = 0 ) uniform Transform
{
mat4 transform;
mat4 cameraTransform;
mat4 projection;
mat4 bckgTransform;
mat4 legLeftTransform;
mat4 legRightTransform;
mat4 cabinTransform;
vec2 scale;
}tform;

layout( location = 0 ) in vec4 aVertex;
layout( location = 1 ) in vec4 aNormal;
layout( location = 2 ) in vec4 aColor;
layout( location = 3 ) in vec4 aTexCoord;

layout ( location = 0 ) out vec2 vTexCoord;

void main() {

vTexCoord=vec2(aTexCoord.x,aTexCoord.y);

gl_Position = tform.projection * tform.cameraTransform * tform.cabinTransform  *  (vec4(aVertex.xyz,1.0));
}