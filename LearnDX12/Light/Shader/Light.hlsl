#include "DSMCG.hlsli"
#include "DSMLighting.hlsli"

// SM 5.1 引进的新语法
ConstantBuffer<ObjectConstants> gObjCB : register(b0);
ConstantBuffer<PassConstants> gPassCB : register(b1);
ConstantBuffer<Lights> gLightCB : register(b2);
ConstantBuffer<MaterialConstants> gMatCB : register(b3);

VertexPosWHNormalW VS(VertexPosLNormalL v)
{
    VertexPosWHNormalW o;
    float4x4 viewProj = mul(gPassCB.View, gPassCB.Proj);
    float4 posW = mul(float4(v.PosL, 1), gObjCB.World);
    o.PosH = mul(posW, viewProj);
    o.PosW = posW.xyz;
    o.NormalW = mul(float4(v.NormalL, 1), gObjCB.WorldInvTranspos).xyz;
    return o;
}

float4 PS(VertexPosWHNormalW i) : SV_Target
{
    float3 viewDir = normalize(i.PosW - gPassCB.EyePosW);
    float3 normal = normalize(i.NormalW);
    
    float shadowFactor[MAXDIRLIGHTCOUNT];
    [unloop]
    for (int index = 0; index < MAXDIRLIGHTCOUNT; ++index)
    {
        shadowFactor[index] = 1;
    }
    //Lights lights = gLightCB;
    //MaterialConstants mat = gMatCB;
    MaterialConstants mat;
    mat.Diffuse = float3(1, 1, 1);
    mat.Specular = float3(0.5, 0.5, 0.5);
    mat.Ambient = float3(0.05, 0.05, 0.05);
    mat.Alpha = 1;
    mat.Gloss = 0.25;
    float3 col = ComputeLighting(gLightCB, mat, viewDir, normal, i.PosW, shadowFactor);
    col += gMatCB.Ambient;

    return float4(col, gMatCB.Alpha);
}