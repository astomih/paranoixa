#version 450 

layout(location=0) in vec3 inColor;
layout(location=1) in vec2 inUV;

layout(location=0) out vec4 outColor;

#ifdef VULKAN
layout(set=2,binding = 0) uniform sampler2D tex;
#else
layout(set=0, binding = 0) uniform texture2D tex;
layout(set=0, binding = 1) uniform sampler samp;
#endif

void main()
{
  vec2 f = vec2(inUV.x, 1.0 - inUV.y);
  f = inUV;
  #ifdef VULKAN
 outColor = texture(tex, f);
  #else
  outColor = texture(sampler2D(tex, samp), f);
  #endif
}