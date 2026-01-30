// Shader.hlsl
#pragma pack_matrix(row_major)

cbuffer CBPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.position = mul(float4(input.position, 1.0f), gWorldViewProj);
    output.uv = input.uv;
    output.color = input.color;
    
    return output;
}

Texture2D gTex : register(t0);
SamplerState gSamp : register(s0);

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return gTex.Sample(gSamp, input.uv);
}