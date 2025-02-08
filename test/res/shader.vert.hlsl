#pragma target ps_6_6
struct VSInput
{
  float3 Position : POSITION;
  float2 TexCoord : TEXCOORD;
  float3 Color : COLOR;
};
struct VSOutput
{
  float4 Position : SV_POSITION;
  float2 TexCoord : TEXCOORD;
  float3 Color : COLOR;
};

VSOutput main( VSInput In )
{
  VSOutput result = (VSOutput)0;
  result.Position = In.Position;
  result.TexCoord = In.TexCoord;
  result.Color = In.Color;
  return result;
}
