#define M_PI 3.141592

cbuffer CBChangesEveryDraw : register(b0)
{
    float4 g_Color;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_WorldViewProj;
}

cbuffer CBPatchTess : register(b2)
{
    float4 g_QuadEdgeTess;
    float2 g_QuadInsideTess;
}

struct VertexPos
{
    float3 posL : POSITION;
};

struct VertexPosF4
{
    float4 posL : POSITION;
};

struct QuadPatchTess
{
    float edgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

QuadPatchTess QuadConstantHS(InputPatch<VertexPosF4, 2> patch, uint patchID : SV_PrimitiveID)
{
    QuadPatchTess pt;
    
    pt.edgeTess[0] = g_QuadEdgeTess[0];
    pt.edgeTess[1] = g_QuadEdgeTess[1];
    pt.edgeTess[2] = g_QuadEdgeTess[2];
    pt.edgeTess[3] = g_QuadEdgeTess[3];
    pt.InsideTess[0] = g_QuadInsideTess[0];
    pt.InsideTess[1] = g_QuadInsideTess[1];
    
    return pt;
}
