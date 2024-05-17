#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

//输入布局描述
const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0 ,0 ,D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init()) {
        return false;
    }
    if (!InitEffect()) {
        return false;
    }
    if (!InitResource()) {
        return false;
    }
    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.00002f;
    theta += 0.000025f;
    m_CBuffer.world = XMMatrixTranspose(XMMatrixRotationX(phi) * XMMatrixRotationY(theta));

    D3D11_MAPPED_SUBRESOURCE mappedData;
    m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    memcpy_s(mappedData.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float pink[4] = { 255.0f / 255.0f,192.0f / 255.0f, 203.0f / 255.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&pink));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_pd3dImmediateContext->DrawIndexed(18, 0, 0);

    m_pd3dImmediateContext->DrawIndexed(36, 18, 5);

    HR(m_pSwapChain->Present(0, 0));
}

//用于初始化着色器或相关特效
bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    //读取编译好的二进制文件
    HR(D3DReadFileToBlob(L"HLSL Files\\Cube_VS.cso", blob.GetAddressOf()));
    //创建顶点着色器
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    //创建顶点输入布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));


    //读取编译好的二进制文件
    HR(D3DReadFileToBlob(L"HLSL Files\\Cube_PS.cso", blob.GetAddressOf()));
    //创建像素着色器
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

    return true;
}

//初始化缓冲区和图元类型等
bool GameApp::InitResource()
{
    //设置四棱锥 
    //    / \4  
    //   /   \
    //  /     \
    // /3_______\2
    // \        \
    // 0\________\1
    //四棱锥顶点数据
    VertexPosColor vertices2[] = {
        {XMFLOAT3(-4.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-2.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-2.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f)},
        {XMFLOAT3(-4.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-3.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)},

         { XMFLOAT3(2.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(4.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(4.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(2.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(4.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(4.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }

    };

    // 设置立方体顶点
    //    5________ 6
    //    /|      /|
    //   /_|_____/ |
    //  1|4|_ _ 2|_|7
    //   | /     | /
    //   |/______|/
    //  0       3
    //顶点数据
    //VertexPosColor vertices1[] = {
    //    { XMFLOAT3(4.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
    //    { XMFLOAT3(4.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
    //    { XMFLOAT3(6.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
    //    { XMFLOAT3(4.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
    //    { XMFLOAT3(4.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
    //    { XMFLOAT3(6.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
    //};

    //VertexPosColor vertices[] =
    //{
    //     { XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    //     { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    //     { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    //};

    //创建缓冲区

    //创建顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));  //将缓冲区数据置零
    vbd.ByteWidth = sizeof(vertices2);
    vbd.Usage = D3D11_USAGE_IMMUTABLE;  //设置gpu只读权限
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;   //设置标志为顶点缓冲区
    vbd.CPUAccessFlags = 0;

    //创建顶点子资源
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices2;

    //创建顶点缓冲区
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    //在输入装配阶段绑定顶点缓冲区
    UINT strides = sizeof(VertexPosColor);  //跨越字节数
    UINT offset = 0;    //起始偏移量
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offset);



    //使用索引，减少重复的点，索引数组
    //DWORD indices1[] = {
    //    // 正面
    //    0, 1, 2,
    //    2, 3, 0,
    //    // 左面
    //    4, 5, 1,
    //    1, 0, 4,
    //    // 顶面
    //    1, 5, 6,
    //    6, 2, 1,
    //    // 背面
    //    7, 6, 5,
    //    5, 4, 7,
    //    // 右面
    //    3, 2, 6,
    //    6, 7, 3,
    //    // 底面
    //    4, 0, 3,
    //    3, 7, 4
    //};

    //设置四棱锥索引
    DWORD indices2[] = {
        0,2,3,
        0,1,2,
        0,4,1,
        1,4,2,
        3,4,0,
        2,4,3,

        // 正面
        0, 1, 2,
        2, 3, 0,
        // 左面
        4, 5, 1,
        1, 0, 4,
        // 顶面
        1, 5, 6,
        6, 2, 1,
        // 背面
        7, 6, 5,
        5, 4, 7,
        // 右面
        3, 2, 6,
        6, 7, 3,
        // 底面
        4, 0, 3,
        3, 7, 4

    };

    //设置索引缓冲区的描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(indices2);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    //建立索引缓冲区
    InitData.pSysMem = indices2;
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndicesBuffer.GetAddressOf()));

    //绑定索引缓冲区
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);



    //常量缓冲区描述
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    //建立常量缓冲区
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

    //绑定常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    //初始化常量缓冲区
    m_CBuffer.world = XMMatrixIdentity();    // 单位矩阵的转置是它本身
    m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));



    //绑定顶点布局
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

    //绑定着色器
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    //设置图元类型
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndicesBuffer.Get(), "IndicesBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

    return true;
}