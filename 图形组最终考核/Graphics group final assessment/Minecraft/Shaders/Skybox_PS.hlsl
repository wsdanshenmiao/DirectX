#include "Skybox.hlsli"

float4 PS(VertexPosHL pIn,uniform float skyCount) : SV_Target
{
    return g_TexCube.Sample(g_Sam, pIn.posL) * float4(skyCount, skyCount, skyCount, 1.0f);
}
