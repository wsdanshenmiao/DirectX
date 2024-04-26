#include "Particle.hlsli"

float4 PS(GeoOut pIn) : SV_Target
{
    return g_TextureInput.Sample(g_SamLinear, pIn.tex);
}
