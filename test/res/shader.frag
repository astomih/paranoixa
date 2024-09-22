#version 450

layout(location=0) in vec3 inColor;
layout(location=0) out vec4 outColor;

layout(binding = 0) uniform sampler2D tex;
void main()
{
  outColor = vec4(inColor, 1) * texture(tex, gl_FragCoord.xy / vec2(800, 600));
}
