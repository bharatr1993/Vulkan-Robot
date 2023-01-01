#version 450

layout (location = 0) in vec3 wVertexPos;
layout ( location = 1 ) in vec3 vN;
layout ( location = 2 ) in vec2 vTexCoord;

layout (location = 0) out vec4 outColor;

layout( set = 0, binding = 0 ) uniform Transform
{
  mat4 transform;
  mat4 normal;
  mat4 projection;
  mat4 bckgTransform;
  vec2 scale;
}tform;

layout( set = 1, binding = 0 ) uniform sampler2D uSampler;

layout( set = 2, binding = 0 ) uniform sampler2D uSampNorm;

layout( set = 3, binding = 0 ) uniform sampler2D uSampRough;

layout( set = 4, binding = 0 ) uniform sampler2D uSampEm;

layout( set = 5, binding = 0 ) uniform sampler2D uSampDs;

const float AMBIENT = 0.85;

void main() {

  vec3 rgb;

  vec3 normTransform = (normalize(vN).rgb);

  vec3 lightVec = normalize(normalize(-vec3(0.0f,0.0f,-3.0f) + wVertexPos));

  vec3 viewVec = normalize(normalize(normTransform-wVertexPos));

  vec3 reflectVec = reflect(lightVec,normTransform);

  float spec = max(dot(reflectVec, viewVec),0);
  
  spec = 0.25*pow(spec,2.0);

  vec3 specContrib = vec3(0.05f,0.05f,0.05f) * spec;

  vec3 ambientContrib = vec3(AMBIENT,AMBIENT,AMBIENT);

  vec3 diffContrib = vec3(1.0f) * max(dot(lightVec,normTransform),0);

  //if(texture( uSampJup, vTexCoord ).rgb!=vec3(1.0f))
    outColor = vec4( (ambientContrib) * texture( uSampDs, vTexCoord ).rgb,1.0f);
  //else
  //outColor=vec4(0.0f);
 // outColor =vec4(0.0f,0.0f,1.0f,1.0f);
}