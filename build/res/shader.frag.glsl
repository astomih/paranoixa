#version 450
layout(row_major) uniform;
layout(row_major) buffer;
layout(binding = 0, set = 2)
uniform sampler2D sampledTexture_0;

layout(location = 0)
out vec4 entryPointParam_FSmain_0;

layout(location = 1)
in vec2 input_uv_0;

void main()
{
    entryPointParam_FSmain_0 = (texture((sampledTexture_0), (input_uv_0)));
    return;
}

