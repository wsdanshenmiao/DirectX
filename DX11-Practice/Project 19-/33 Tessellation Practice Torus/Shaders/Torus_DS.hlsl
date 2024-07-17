#include "Torus.hlsli"

[domain("quad")]
float4 DS(QuadPatchTess patchTess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<VertexPos, 5> quad) : SV_POSITION
{
    /*
   0-----1
    |   |
    |   |
   3-----2
    */
    float3 normal = quad[3].posL;
    float3 origin = quad[4].posL;
    float3 v10 = quad[0].posL - quad[1].posL;
    float3 v12 = quad[2].posL - quad[1].posL;
    float3 v3 = quad[1].posL + v10 + v12;    // ����ĸ��� 
    // ˫���Բ�ֵ
    float3 v1 = lerp(quad[0].posL, quad[1].posL, uv.x);
    float3 v2 = lerp(v3, quad[2].posL, uv.x);
    float3 p = lerp(v1, v2, uv.y);

    // ��ȡ��ǰ����
    float length = sqrt(pow(v10.x, 2) + pow(v10.y, 2) + pow(v10.z, 2));
    float weight = sqrt(pow(v12.x, 2) + pow(v12.y, 2) + pow(v12.z, 2));
    float torusR = weight / (2 * M_PI);
    float r = length / (2 * M_PI) - torusR;
    float R = 2 * torusR + r;

    float theta = p.y / torusR; // ������
    p = float3(p.x, sin(theta) * torusR, cos(theta) * torusR);  // �����

    float newR = r + torusR * 2 * (p.y + weight / 2) / weight;
    theta = p.x / r;
    p = float3(cos(M_PI / 2 - theta) * newR, sin(M_PI / 2 - theta) * newR, p.z);    // ���ɻ�

    // ��y����ת
    theta = acos(normal.y); // ��y��ļн�
    float lengthxz = sin(theta);    //xzƽ��ͶӰ�ĳ���
    theta = acos(normal.z / lengthxz);
    float3x3 rotate1 = float3x3(
        float3(cos(theta), 0, -sin(theta)),
        float3(0, 1, 0),
        float3(sin(theta), 0, cos(theta))
    );

    // ��z����ת 
    theta = acos(normal.z);
    float lengthxy = sin(theta);
    theta = acos(normal.x / lengthxy);
    float3x3 rotate2 = float3x3(
        float3(cos(theta), sin(theta), 0),
        float3(-sin(theta), cos(theta), 0),
        float3(0, 0, 1)
    );

    float3x3 rotate = mul(rotate2, rotate1);    // ������ת����

    p = mul(rotate, p) + origin;    // �ƶ�������λ����Ҫ������ɫ���н���

    float4 posH = mul(float4(p, 1.0f), g_WorldViewProj);
    //float4 posH = float4(p,1.0f);

    return posH;
}