#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "LightHelper.h"
#include "Geometry.h"

class GameApp : public D3DApp
{
public:

    struct VSConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
        DirectX::XMMATRIX texRotarion;
    };

    struct PSConstantBuffer
    {
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        Material material;
        int numDirLight;
        int numPointLight;
        int numSpotLight;
        float pad;		// 打包保证16字节对齐
        DirectX::XMFLOAT4 eyePos;
    };

    enum class ShowMode { WoodCrate, FireBox };

public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();


private:
    bool InitEffect();
    bool InitResource();

    template <typename VertexType>
    bool ResetMesh(const Geometry::MeshData<VertexType>& meshData);


private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // 顶点输入布局

    ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 顶点缓冲区
    ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
    ComPtr<ID3D11Buffer> m_pConstantBuffers[2];	    // 常量缓冲区

    UINT m_IndexCount;							    // 绘制物体的索引数组大小

    ComPtr<ID3D11VertexShader> m_pVertexShader;	    // 顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader;		// 像素着色器

    VSConstantBuffer m_VSConstantBuffer;			// 用于修改用于VS的GPU常量缓冲区的变量
    PSConstantBuffer m_PSConstantBuffer;			// 用于修改用于PS的GPU常量缓冲区的变量

    ComPtr<ID3D11SamplerState> m_pSamplerState;		// 采样器状态

    ComPtr<ID3D11ShaderResourceView> m_pWoodCrate;  //木箱纹理 
    ComPtr<ID3D11ShaderResourceView> m_pFlare;      //火焰箱纹理 
    ComPtr<ID3D11ShaderResourceView> m_pFlarealpha; //火焰箱纹理 
    ShowMode m_CurrMode;							// 当前显示的模式


    DirectionalLight m_DirLight;					// 默认环境光
    PointLight m_PointLight;						// 默认点光
    SpotLight m_SpotLight;						    // 默认汇聚光

    ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
    bool m_IsWireframeMode;							// 当前是否为线框模式
    
};


#endif