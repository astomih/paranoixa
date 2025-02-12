// shaders.slang

struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
};

struct VSOutput
{
    float4 out : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

[shader("vertex")]
VSOutput VSmain(VSInput input)
{
    VSOutput output;
    output.out = float4(input.pos, 1.f);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

struct FSInput
{
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

#define COMBINED_IMAGE_SAMPLER(name, bind, set) \
    [[vk::binding(bind, set)]]                  \
    Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

[shader("fragment")]
float4 FSmain(FSInput input)
    : SV_Target
{
    return sampledTexture.Sample(input.uv);
}
