#include "Basic.hlsli"

// 顶点着色器
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);
    float3 normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    // 若当前在绘制反射物体，先进行反射操作
    [flatten]
    if (g_IsReflection)
    {
        posW = mul(posW, g_Reflection);
        normalW = mul(normalW, (float3x3) g_Reflection);
    }
    // 若当前在绘制阴影，先进行投影操作
    [flatten]
    if (g_IsShadow)
    {
        posW = (g_IsReflection ? mul(posW, g_RefShadow) : mul(posW, g_Shadow));
    }

    vOut.posH = mul(posW, g_ViewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = normalW;
    vOut.tex = vIn.tex;
    return vOut;
}