#version 450

layout( set = 0, binding = 0 ) uniform Transform
{
  mat4 transform;
  mat4 ringtransform;
  mat4 consttransform;
  mat4 normal;
  mat4 ringNormal;
  mat4 projection;
  mat4 textransform;
  mat4 aimtransform;
  mat4 dstransform;
  mat4 juptransform;
  mat4 jupnormal;
  mat4 asdtransform;
  mat4 asdnormal;
  float time;
}tform;

const float AMBIENT = 0.2;

layout( location = 0 ) in vec4 aVertex;
layout( location = 1 ) in vec4 aNormal;
layout( location = 2 ) in vec4 aColor;
layout( location = 3 ) in vec4 aTexCoord;

layout ( location = 0 ) out vec3 wVertexPos;
layout ( location = 1 ) out vec2 vTexCoord;
layout ( location = 2 ) out vec3 vN;


void main() {

  vec4 worldVertexPos = tform.juptransform * vec4(aVertex.xyz,1.0);
  wVertexPos = worldVertexPos.xyz;
  vN = normalize(tform.jupnormal * aNormal).xyz;
  vTexCoord=vec2(aTexCoord.x,aTexCoord.y);
  gl_Position = tform.projection * tform.juptransform * vec4(aVertex.xyz, 1.0);
}