struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Color : COLOR;
};
Texture2D gTex : register(t0);
SamplerState gSampler : register(s0);
float4 main(VSOutput In) : SV_TARGET
{
		In.TexCoord.y = 1.0 - In.TexCoord.y;
    return gTex.Sample(gSampler, In.TexCoord);
}
