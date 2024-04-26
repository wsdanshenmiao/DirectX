#include "Particle.hlsli"

[maxvertexcount(6)]
void GS(point VertexOut gIn[1], inout LineStream<GeoOut> output)
{
    // 不要绘制用于产生粒子的顶点
    if (gIn[0].Type != PT_EMITTER)
    {
        // 使线段沿着一个加速度方向倾斜
        float3 p0 = gIn[0].posW;
        float3 p1 = gIn[0].posW + 0.07f * g_AccelW;
        
        GeoOut v0;
        v0.posH = mul(float4(p0, 1.0f), g_ViewProj);
        v0.tex = float2(0.0f, 0.0f);
        output.Append(v0);
        
        GeoOut v1;
        v1.posH = mul(float4(p1, 1.0f), g_ViewProj);
        v1.tex = float2(0.0f, 0.0f);
        output.Append(v1);
    }
}
