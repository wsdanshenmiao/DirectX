#include "Torus.hlsli"

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("QuadConstantHS")]
[maxtessfactor(64.0f)]
float3 HS( 
	InputPatch<VertexPosF4, 2> patch, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID ) : POSITION
{
    float3 newPoint[4];

	float3 origin = patch[0].posL.xyz;
    float3 normal = patch[1].posL.xyz;  // Բ��ƽ��ķ�����
    float r = patch[0].posL.w;
    float R = patch[1].posL.w;
 
    float torusR = (R - r) / 2; //Բ���뾶
    float length = 2 * M_PI * (r + torusR);
    float weight = 2 * M_PI * torusR; 
    /*
   0-----1
    |   |
    |   |
   3-----2
    */
    newPoint[0] = float3(0, weight, 0);  // ����ԭ���Ϊ��������
    newPoint[1] = float3(length, weight, 0);
    newPoint[2] = float3(length, 0, 0);
    newPoint[3] = normalize(normal);   // ���ݷ�����

    return newPoint[i];
}