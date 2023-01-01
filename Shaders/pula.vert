#version 450

layout( set = 0, binding = 0 ) uniform Transform
{
  mat4 transform;
  mat4 normal;
  mat4 projection;
  mat4 bckgTransform;
  vec2 scale;
}tform;

const float AMBIENT = 0.2;

layout( location = 0 ) in vec4 aVertex;
layout( location = 1 ) in vec4 aNormal;
layout( location = 2 ) in vec4 aColor;
layout( location = 3 ) in vec4 aTexCoord;

layout( location = 4 ) in vec2 centerPosition;
layout( location = 5 ) in vec2 centerVelocity;
layout( location = 6 ) in vec2 centerRadius;
layout( location = 7 ) in vec2 centerOffset;

layout ( location = 0 ) out vec3 wVertexPos;
layout ( location = 1 ) out vec3 vN;
layout ( location = 2 ) out vec2 vTexCoord;

mat4 BuildTranslationScale(vec3 delta, float scale)
{
  return mat4(
  vec4(scale, 0.0, 0.0, 0.0),
  vec4(0.0, scale, 0.0, 0.0),
  vec4(0.0, 0.0, scale, 0.0),
  vec4(delta, 1.0));
}

void main() {

  mat4 tf=BuildTranslationScale(vec3(vec2(centerPosition.x+0.05*centerOffset.x,centerPosition.y+0.05*centerOffset.y),0.0f),centerRadius.x);
  vec4 worldVertexPos = tf*tform.transform*vec4(aVertex.xyz,1.0);
  wVertexPos = worldVertexPos.xyz;
  vN = normalize(inverse(transpose(tf))*tform.normal * aNormal).xyz;
  vTexCoord=vec2(aTexCoord.x,aTexCoord.y);

  gl_Position =     tf * tform.projection * tform.transform  *  (vec4(aVertex.xyz,1.0));

}