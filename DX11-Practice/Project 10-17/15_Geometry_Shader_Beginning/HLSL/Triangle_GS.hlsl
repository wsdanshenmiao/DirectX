#include "Basic.hlsli"

[maxvertexcount(12)]
void GS(triangle VertexPosHColor input[3], inout TriangleStream<VertexPosHColor> output)
{
    // 将一个三角形分裂成三个三角形，即没有v3v4v5的三角形
    //       v1
    //       /\
    //      /  \
    //   v3/____\v4
    //    /\xxxx/\
    //   / \ x  / \
    //  /__\___/___\
    // v0  v6   v5   v2

    VertexPosHColor outputPoint[4];

    int i;
    [unroll]
    for(i =0;i<3;++i){
        outputPoint[i].posH = (input[i].posH+input[(i+1)%3].posH)/2.0f;
        outputPoint[i].color = (input[i].color+input[(i+1)%3].color)/2.0f;
    }

    outputPoint[3].posH=(input[0].posH+outputPoint[2].posH)/2.0f;
    outputPoint[3].color=(input[0].color+outputPoint[2].color)/2.0f;
    outputPoint[2].posH = (input[2].posH+outputPoint[2].posH)/2.0f;
    outputPoint[2].color = (input[2].color+outputPoint[2].color)/2.0f;

    [unroll]
    for(i=0;i<3;++i){
        output.Append(input[i]);
        output.Append(outputPoint[i]);
        output.Append(outputPoint[(i+3)%4]);
        output.RestartStrip();
    }
}