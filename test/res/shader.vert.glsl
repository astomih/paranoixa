#version 450
layout(row_major) uniform;
layout(row_major) buffer;
layout(location = 0)
out vec3 entryPointParam_VSmain_color_0;

layout(location = 1)
out vec2 entryPointParam_VSmain_uv_0;

layout(location = 0)
in vec3 input_pos_0;

layout(location = 1)
in vec2 input_uv_0;

layout(location = 2)
in vec3 input_color_0;

struct VSOutput_0
{
    vec4 out_0;
    vec3 color_0;
    vec2 uv_0;
};

void main()
{
    VSOutput_0 output_0;
    output_0.out_0 = vec4(input_pos_0, 1.0);
    output_0.uv_0 = input_uv_0;
    output_0.color_0 = input_color_0;
    VSOutput_0 _S1 = output_0;
    gl_Position = output_0.out_0;
    entryPointParam_VSmain_color_0 = _S1.color_0;
    entryPointParam_VSmain_uv_0 = _S1.uv_0;
    return;
}

