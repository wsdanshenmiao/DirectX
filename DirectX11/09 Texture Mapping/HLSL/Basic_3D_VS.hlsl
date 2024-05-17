#include "Basic.hlsli"

// 顶点着色器
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
#if 0
    vOut.tex = vIn.tex * 2;
#else
    //要移动到中心后在旋转
    vOut.tex = mul(float4(vIn.tex.x - 0.5f, vIn.tex.y - 0.5f, 0.0f, 1.0f), g_texRotarion).xy;
#endif
    //返回原位
    vOut.tex += 0.5;
    
    return vOut;
}