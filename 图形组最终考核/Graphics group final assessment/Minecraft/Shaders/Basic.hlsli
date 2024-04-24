#include "LightHelper.hlsli"

Texture2D g_Tex : register(t0);
Texture2DArray g_TexArray : register(t1);
Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);


cbuffer CBChangesEveryInstanceDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
}

cbuffer CBChangesEveryObjectDrawing : register(b1)
{
    Material g_Material;
}

cbuffer CBDrawingStates : register(b2)
{
    int g_IsReflection;
    int g_IsShadow;
}

cbuffer CBDrawingStates : register(b3)
{
    float4 g_FogColor;
    int g_FogEnabled;
    float g_FogStart;
    float g_FogRange;
    int g_Pad;
}

cbuffer CBChangesEveryFrame : register(b4)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
    float g_Pad2;
}

cbuffer CBChangesOnResize : register(b5)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b6)
{
    matrix g_Reflection;
    matrix g_Shadow;
    matrix g_RefShadow;
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float2 tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 posH : SV_POSITION;
    float2 tex : TEXCOORD;
};

struct InstancePosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
    matrix world : World;
    matrix worldInvTranspose : WorldInvTranspose;
};

struct BillboardVertex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 tex : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

struct PointSprite
{
    float3 posW : POSITION;
    float2 SizeW : SIZE;
};
