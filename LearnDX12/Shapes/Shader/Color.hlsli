
struct VertexPosLColor
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexPosHColor
{
    float4 PosH : SV_Position;
    float4 Color : COLOR;
};

// 常量缓冲区的大小必须为 256 b 的整数倍，隐式填充为 256 byte
cbuffer CBPerObject : register(b0)
{
    float4x4 gWorldViewProj;
}

//// SM 5.1 引进的新语法
//struct ObjectConstants
//{
//    float4x4 gWorldViewProj;
//};
//ConstantBuffer<ObjectConstants> gObjConstants : register(b0);
