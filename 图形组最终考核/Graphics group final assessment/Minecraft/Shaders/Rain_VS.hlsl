#include "Particle.hlsli"

VertexOut VS(VertexParticle vIn)
{
    VertexOut vOut;
    
    float t = vIn.age;
    
    // 恒定加速度等式
    vOut.posW = 0.5f * t * t * g_AccelW + t * vIn.initialVelW + vIn.initialPosW;
    
    vOut.Type = vIn.type;
    
    return vOut;
}
