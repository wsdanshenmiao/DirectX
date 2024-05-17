#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"

class GameApp : public D3DApp
{
public:
    //常量缓冲区
    struct ConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT4 color;
        uint32_t useCustomColor;
    };

    //对应HLSL中的顶点
    struct VertexPosColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;  //顶点输入布局
    ComPtr<ID3D11VertexShader> m_pVertexShader; //顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader;   //像素着色器 

    ComPtr<ID3D11Buffer> m_pVertexBuffer1;   //顶点缓冲区
    ComPtr<ID3D11Buffer> m_pVertexBuffer2;   //顶点缓冲区

    ComPtr<ID3D11Buffer> m_pIndicesBuffer1;  //索引缓冲区
    ComPtr<ID3D11Buffer> m_pIndicesBuffer2;  //索引缓冲区

    ComPtr<ID3D11Buffer> m_pConstantBuffer;    //常量缓冲区

    ConstantBuffer m_CBuffer;   //用于修改GPU常量缓冲区的变量

public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
    bool InitEffect();  //用于初始化着色器或相关特效
    bool InitResource(); //初始化缓冲区和图元类型等
    void ShowImGuiWindow(); //绘制ImGui窗口
};


#endif