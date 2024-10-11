#version 450

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inUV;
layout(location=2) in vec3 inColor;

layout(location=0) out vec3 outColor;
layout(location=1) out vec2 outUV;

void main()
{
  gl_Position = vec4(inPosition, 1);
  outColor = inColor;
  outUV = inUV;
}
