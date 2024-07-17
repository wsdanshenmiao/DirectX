#include "Basic.hlsli"

// 一个v0v1线段输出6个三角形顶点
[maxvertexcount(6)]
void GS(line VertexPosHWNormalColor input[2], inout TriangleStream<VertexPosHWNormalColor> output)
{
    // ***************************
    // 要求圆线是顺时针的，然后自底向上构造圆柱侧面           
    //   -->      v3____v2
    //  ______     |\   |
    // /      \    | \  |
    // \______/    |  \ |
    //   <--       |___\|
    //           v1(i1) v0(i0)

    VertexPosHWNormalColor outputPoint[2];

    float3 lineVectorW = input[1].posW - input[0].posW;
    float3 extensionDir = normalize(cross(input[0].normalW,lineVectorW));

    matrix viewProj = mul(g_View,g_Proj);

    [unroll]
    for(uint i=0;i<2;++i){
        outputPoint[i].posW=input[i].posW+extensionDir*g_CylinderHeight;
        outputPoint[i].posH=mul(float4(outputPoint[i].posW,1.0f),viewProj);
        outputPoint[i].normalW=input[i].normalW;
        outputPoint[i].color=input[i].color;
    }

    output.Append(input[0]);
    output.Append(outputPoint[1]);
    output.Append(outputPoint[0]);
    output.RestartStrip();
    output.Append(input[1]);
    output.Append(outputPoint[1]);
    output.Append(input[0]);
    output.RestartStrip();
}