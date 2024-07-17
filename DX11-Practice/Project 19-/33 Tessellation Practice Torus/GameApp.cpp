#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>
#include <DirectXColors.h>
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_pEffectHelper(std::make_unique<EffectHelper>())
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    if (!InitResource())
        return false;

    if (!InitEffect())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    if (ImGui::Begin("Torus"))
    {
        UpdataTorus();
    }

    ImGui::End();
    ImGui::Render();
}

void GameApp::UpdataTorus()
{
    static bool mouseControl = true;
    ImGui::Checkbox("Mouse Control", &mouseControl);
    if (!mouseControl) {
        ImGui::SliderFloat3("Normal", m_Normal, -5, 5);
    }
    ImGui::SliderFloat3("Center", m_Center, -5, 5);


    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (mouseControl) {
            m_Theta = XMScalarModAngle(m_Theta + io.MouseDelta.x * 0.01f);
            m_Phi += -io.MouseDelta.y * 0.01f;
        }
    }
    m_Radius -= io.MouseWheel * 0.1f;

    // 限制Phi
    //m_Phi = std::clamp(m_Phi, XM_PI / 18, 1.0f - XM_PI / 18);
    // 限制半径
    m_Radius = std::clamp(m_Radius, 0.5f, 100.0f);

    XMVECTOR posVec = XMVectorSet(
        m_Radius * sinf(m_Phi) * cosf(m_Theta),
        m_Radius * cosf(m_Phi),
        m_Radius * sinf(m_Phi) * sinf(m_Theta),
        0.0f);

    // *****************
    // 更新数据并应用
    //
    XMMATRIX WVP = XMMatrixLookAtLH(posVec, g_XMZero, g_XMIdentityR1) *
        XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 0.1f, 1000.0f);
    WVP = XMMatrixTranspose(WVP);

    XMFLOAT4 torusVertices[2] = {
        XMFLOAT4(m_Center[0],m_Center[1],m_Center[2],0.5f),  // 原点位置和小圈半径
        XMFLOAT4(m_Normal[0],m_Normal[1],m_Normal[2],1.0f)   // 圆环平面的法线单位向量和大圈半径
    };

    if (!mouseControl) {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(m_pd3dImmediateContext->Map(m_pTorusBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(reinterpret_cast<XMFLOAT3*>(mappedData.pData), sizeof(torusVertices), torusVertices, sizeof(torusVertices));
        m_pd3dImmediateContext->Unmap(m_pTorusBuffer.Get(), 0);
    }

    ImGui::SliderFloat4("QuadEdgeTess", m_QuadEdgeTess, 20.0f, 50.0f, "%.1f");
    ImGui::SliderFloat2("QuadInsideTess", m_QuadInsideTess, 20.0f, 50.0f, "%.1f");
    m_pEffectHelper->GetConstantBufferVariable("g_QuadEdgeTess")->SetFloatVector(4, m_QuadEdgeTess);
    m_pEffectHelper->GetConstantBufferVariable("g_QuadInsideTess")->SetFloatVector(2, m_QuadInsideTess);

    // 更新数据并应用
    m_pEffectHelper->GetConstantBufferVariable("g_WorldViewProj")->SetFloatMatrix(4, 4, (const FLOAT*)&WVP);
    m_pEffectHelper->GetConstantBufferVariable("g_QuadEdgeTess")->SetFloatVector(4, m_QuadEdgeTess);
    m_pEffectHelper->GetConstantBufferVariable("g_QuadInsideTess")->SetFloatVector(2, m_QuadInsideTess);
}

void GameApp::DrawScene()
{
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }
    
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), Colors::Black);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    UINT strides[1] = { sizeof(XMFLOAT4) };
    UINT offsets[1] = { 0 };
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pTorusBuffer.GetAddressOf(), strides, offsets);
    m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST);

    m_pEffectHelper->GetConstantBufferVariable("g_Color")->SetFloatVector(4, Colors::White);
    m_pEffectHelper->GetEffectPass("Torus")->Apply(m_pd3dImmediateContext.Get());
    m_pd3dImmediateContext->Draw(2, 0);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
    // ******************
    // 创建缓冲区
    //
    XMFLOAT4 torusVertices[2] = {
        XMFLOAT4(0,0,0,0.5f),  // 原点位置和小圈半径
        XMFLOAT4(3,4,5,1.0f)   // 圆环平面的法线单位向量和大圈半径
    };
    CD3D11_BUFFER_DESC bufferDesc(sizeof torusVertices, D3D11_BIND_VERTEX_BUFFER);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA initData{ torusVertices };
    HR(m_pd3dDevice->CreateBuffer(&bufferDesc, &initData, m_pTorusBuffer.GetAddressOf()));

    // ******************
    // 创建光栅化状态
    //
    CD3D11_RASTERIZER_DESC rasterizerDesc(CD3D11_DEFAULT{});
    // 线框模式
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireFrame.GetAddressOf()));

    return true;
}

bool GameApp::InitEffect()
{
    // ******************
    // 创建着色器和顶点输入布局
    //

    ComPtr<ID3DBlob> blob;
    D3D11_INPUT_ELEMENT_DESC inputElemDesc[1] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    m_pEffectHelper->SetBinaryCacheDirectory(L"Shaders\\Cache");

    HR(m_pEffectHelper->CreateShaderFromFile("Torus_VS", L"Shaders\\Torus_VS.hlsl", m_pd3dDevice.Get(), "VS", "vs_5_0", nullptr, blob.GetAddressOf()));
    HR(m_pd3dDevice->CreateInputLayout(inputElemDesc, 1, blob->GetBufferPointer(), blob->GetBufferSize(), m_pInputLayout.GetAddressOf()));

    HR(m_pEffectHelper->CreateShaderFromFile("Torus_PS", L"Shaders\\Torus_PS.hlsl", m_pd3dDevice.Get(), "PS", "ps_5_0"));

    HR(m_pEffectHelper->CreateShaderFromFile("Torus_HS", L"Shaders\\Torus_HS.hlsl", m_pd3dDevice.Get(), "HS", "hs_5_0"));

    HR(m_pEffectHelper->CreateShaderFromFile("Torus_DS", L"Shaders\\Torus_DS.hlsl", m_pd3dDevice.Get(), "DS", "ds_5_0"));

    EffectPassDesc passDesc;
    passDesc.nameVS = "Torus_VS";
    passDesc.nameHS = "Torus_HS";
    passDesc.nameDS = "Torus_DS";
    passDesc.namePS = "Torus_PS";
    HR(m_pEffectHelper->AddEffectPass("Torus", m_pd3dDevice.Get(), &passDesc));
    m_pEffectHelper->GetEffectPass("Torus")->SetRasterizerState(m_pRSWireFrame.Get());

    m_pEffectHelper->SetDebugObjectName("Tessellation");

    return true;
}

