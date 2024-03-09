#include "Cube.hlsli"

float4 PS(VertexIn vIn) : SV_TARGET
{
    return vIn.color;
}