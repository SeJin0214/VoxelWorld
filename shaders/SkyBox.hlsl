// SkyBox.hlsl

cbuffer CBPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct PS_INPUT
{
    float3 dir : TEXCOORD0;
    float4 position : SV_POSITION;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.dir = input.position;
    output.position = mul(float4(input.position, 1.0f), gWorldViewProj);

    return output;
}

TextureCube gTex : register(t0);
SamplerState gSamp : register(s0);

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // 큐브맵은 단순 방향 벡터로 판단함
    return gTex.Sample(gSamp, input.dir);
}
