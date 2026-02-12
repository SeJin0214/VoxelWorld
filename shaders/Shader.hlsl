// Shader.hlsl

cbuffer CBPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 instancePos : INSTANCEPOS;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = float4(input.position + input.instancePos, 1.0f);
    output.position = mul(output.position, gWorldViewProj);
    output.uv = input.uv;
    
    return output;
}

Texture2D gTex : register(t0);
SamplerState gSamp : register(s0);

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return gTex.Sample(gSamp, input.uv);
}

//float4 mainPS(PS_INPUT input, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
//{
//    return isFrontFace ? gTex.Sample(gSamp, input.uv) : float4(1, 0, 0, 1);
//}
