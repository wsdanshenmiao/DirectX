#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "Geometry.h"
#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight), 
    m_IndexCount(),
    m_VSConstantBuffer(),
    m_PSConstantBuffer(),
    m_DirLight(),
    m_PointLight(),
    m_SpotLight(),
    m_IsWireframeMode(false),
    m_CurrMode(ShowMode::WoodCrate)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    static bool materialAndLight = false;
    static bool useAnisotropic = false;
    static bool animateCube = true; //物体运动

    static float texphi;
    texphi += 3.0f * dt;
    static float phi = 0.0f, theta = 0.0f;
    static float x = 0.0f, y = 0.0f;
    static float fov = XM_PIDIV2;
    static float scale = 1.0f;
    static float length = 3.0;
    static float width = 3.0;
    static float height = 3.0;

    static D3D11_FILTER boxFilter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    static D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_CLAMP;

    //获取IO
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::Begin("09 Texture Mapping"))
    {   
        static int curr_mode_item = static_cast<int>(m_CurrMode);
        const char* mode_strs[] = {
            "WoodCrate",
            "FireBox"
        };
        if (ImGui::Combo("Mode", &curr_mode_item, mode_strs, ARRAYSIZE(mode_strs))) {
            if (0 == curr_mode_item) {
                m_CurrMode = ShowMode::WoodCrate;
                m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
                m_pd3dImmediateContext->PSSetShaderResources(1, 1, m_pWoodCrate.GetAddressOf());
            }
            else {
                m_CurrMode = ShowMode::FireBox;
                m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pFlare.GetAddressOf());
                m_pd3dImmediateContext->PSSetShaderResources(1, 1, m_pFlarealpha.GetAddressOf());
            }
        }

        if (ImGui::Button("Reset Params")) {
            x = y = phi = theta = 0.0f;
            scale = 1.0f;
            fov = XM_PIDIV2;
            length = 3.0;
            width = 3.0;
            height = 3.0;

            m_PointLight.position = XMFLOAT3(-10.0f, 2.0f, -10.0f);
            m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
            m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
            m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
            m_PointLight.att = XMFLOAT3(0.01f, 0.01f, 0.01f);
            m_PointLight.range = 25.0f;
            m_PSConstantBuffer.pointLight[0] = m_PointLight;

            m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
            m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            m_PSConstantBuffer.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 5.0f);
        }

        ImGui::Checkbox("Animate Cube", &animateCube);
        ImGui::Checkbox("Material And Light", &materialAndLight);
        ImGui::Checkbox("Use Anisotropic", &useAnisotropic);

        ImGui::SliderFloat("Length", &length, 1.0f, 8.0f);
        ImGui::SliderFloat("Width", &width, 1.0f, 8.0f);
        ImGui::SliderFloat("Height", &height, 1.0f, 8.0f);

        ImGui::Text("Scale: %0.3f", scale);
        ImGui::SliderFloat("Scale", &scale, 0.2f, 2.0f);

        ImGui::Text("Phi: %0.3f", XMConvertToDegrees(phi));
        ImGui::SliderFloat("X Axis Rotation", &phi, -XM_PI, XM_PI);

        ImGui::Text("Theta: %0.3f", XMConvertToDegrees(theta));
        ImGui::SliderFloat("Y Axis Rotation", &theta, -XM_PI, XM_PI);

        ImGui::Text("Fov: %0.3f", XMConvertToDegrees(fov));
        ImGui::SliderFloat("Fov", &fov, XM_PIDIV4, XM_PI / 3 * 2);

        //若按下按钮修改光栅化阶段,传入空指针为默认状态
        if (ImGui::Checkbox("Wire Frame Mode",&m_IsWireframeMode)) {
            m_pd3dImmediateContext->RSSetState(m_IsWireframeMode?m_pRSWireframe.Get() : nullptr);
        }
    }

    ImGui::End();

    if (animateCube) {
        phi += 0.3f * dt;
        theta += 0.3f * dt;
        //XMScalarModAngle(float Angle) 将传入的角限制在一定范围内
        phi = XMScalarModAngle(phi);
        theta = XMScalarModAngle(theta);
    }

    if (materialAndLight) {
        if (ImGui::Begin("Material And Light")) {
            ImGui::Text("Material");
            ImGui::PushID(3);
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.material.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.material.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.material.specular.x);
            ImGui::PopID();

            ImGui::Text("Point Light");
            ImGui::InputFloat3("Position", &m_PSConstantBuffer.pointLight[0].position.x);
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.pointLight[0].ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.pointLight[0].diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.pointLight[0].specular.x);
            ImGui::InputFloat("Range", &m_PSConstantBuffer.pointLight[0].range);
            ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.pointLight[0].att.x);
        }
        ImGui::End();
    }

    if (!ImGui::IsAnyItemActive()) {    //操作UI时不对物体进行影响
        // 鼠标左键拖动平移
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            x += io.MouseDelta.x * 0.01f;
            y -= io.MouseDelta.y * 0.01f;
        }
        // 鼠标右键拖动旋转
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            phi -= io.MouseDelta.y * 0.01f;
            theta -= io.MouseDelta.x * 0.01f;
            phi = XMScalarModAngle(phi);
            theta = XMScalarModAngle(theta);
        }
        // 鼠标滚轮缩放
        else if (io.MouseWheel != 0.0f) {
            scale += 0.02f * io.MouseWheel;
            if (scale > 2.0f)
                scale = 2.0f;
            else if (scale < 0.2f)
                scale = 0.2f;
        }
    }

    if (useAnisotropic) {
        boxFilter = D3D11_FILTER_ANISOTROPIC;
    }
    else {
        boxFilter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    }

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = boxFilter;
    sampDesc.AddressU = address;
    sampDesc.AddressV = address;
    sampDesc.AddressW = address;
    sampDesc.BorderColor[0] = 0.0f;
    sampDesc.BorderColor[1] = 0.0f;
    sampDesc.BorderColor[2] = 0.0f;
    sampDesc.BorderColor[3] = 0.0f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    //释放采样器
    m_pSamplerState->Release();
    // 重新创建采样器
    HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));
    //设置采样器
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

    //更改木箱属性
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    auto meshData = Geometry::CreateBox<VertexPosNormalTex>(length, width , height, XMFLOAT4(255.0f / 255.0f, 192.0f / 255.0f, 203.0f / 255.0f, 1.0f));
    ResetMesh(meshData);
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
    m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));  //求逆后转置
    m_VSConstantBuffer.world = XMMatrixTranspose(
        XMMatrixScalingFromVector(XMVectorReplicate(scale)) * W * XMMatrixTranslation(x, y, 0.0f)
    );
    m_VSConstantBuffer.proj = XMMatrixTranspose((XMMatrixPerspectiveFovLH(fov, AspectRatio(), 0.1f, 1000.0f)));

    if (m_CurrMode == ShowMode::FireBox) {
        m_VSConstantBuffer.texRotarion = XMMatrixTranspose(XMMatrixRotationZ(texphi));
    }
    else {
        m_VSConstantBuffer.texRotarion = XMMatrixTranspose(XMMatrixRotationZ(0.0f));
    }

    ImGui::Render();

    // 更新常量缓冲区，让立方体转起来
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);

    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}



void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float pink[4] = { 255.0f / 255.0f,192.0f / 255.0f, 203.0f / 255.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&pink));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    
    // 绘制几何模型
    m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}



bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    // 创建顶点着色器
    HR(CreateShaderFromFile(L"HLSL\\Basic_3D_VS.cso", L"HLSL\\Basic_3D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // 创建并绑定顶点布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // 创建像素着色器
    HR(CreateShaderFromFile(L"HLSL\\Basic_3D_PS.cso", L"HLSL\\Basic_3D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));


    return true;
}

bool GameApp::InitResource()
{
    // ******************
    // 初始化网格模型
    //
    auto meshData = Geometry::CreateBox<VertexPosNormalTex>(3, 3, 3, XMFLOAT4(255.0f / 255.0f, 192.0f / 255.0f, 203.0f / 255.0f, 1.0f));
    ResetMesh(meshData);


    //设置光栅化阶段描述
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.FillMode = D3D11_FILL_WIREFRAME; //填充模式设置为线框填充
    rd.CullMode = D3D11_CULL_NONE;  //无背面裁剪，线框模式下不需要背面裁剪
    rd.FrontCounterClockwise = false;
    rd.DepthBias = 0;
    rd.SlopeScaledDepthBias = 0.0f;
    rd.DepthBiasClamp = 0.0f;
    rd.DepthClipEnable = true;  //允许深度测试裁剪范围外的像素
    rd.ScissorEnable = false;
    rd.MultisampleEnable = false;
    rd.AntialiasedLineEnable = false;

    //创建调整光栅化阶段
    HR(m_pd3dDevice->CreateRasterizerState(&rd, m_pRSWireframe.GetAddressOf()));


    // ******************
    // 设置常量缓冲区描述
    //
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(VSConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // 新建用于VS和PS的常量缓冲区
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));

    cbd.ByteWidth = sizeof(PSConstantBuffer);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

    // 初始化用于VS的常量缓冲区的值
    m_VSConstantBuffer.world = XMMatrixIdentity();			
    m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -6.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 0.1f, 1000.0f));
    m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();
    


    //使用点光源
    m_PointLight.position = XMFLOAT3(-6.0f, 3.0f, -6.0f);
    m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PointLight.att = XMFLOAT3(0.0f, 0.0f, 0.01f);
    m_PointLight.range = 25.0f;

    m_PSConstantBuffer.numDirLight = 0;
    m_PSConstantBuffer.numPointLight = 1;
    m_PSConstantBuffer.numSpotLight = 0;

    m_PSConstantBuffer.pointLight[0] = m_PointLight;

    // 初始化用于PS的常量缓冲区的值
    m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 5.0f);

    // 设置此处的观察位置
    m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

    // 更新PS常量缓冲区资源
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);


    // 初始化木箱纹理
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"..\\Texture\\WoodCrate.dds", nullptr, m_pWoodCrate.GetAddressOf()));

    // 初始化火焰箱纹理
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"..\\Texture\\flare.dds", nullptr, m_pFlare.GetAddressOf()));
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"..\\Texture\\flarealpha.dds", nullptr, m_pFlarealpha.GetAddressOf()));

    // 初始化采样器状态
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    //使用点，点，点采样
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    //使用点，点，线采样
    //sampDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    //使用点，线，点采样
    //sampDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    //全部使用线性采样
    //sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    //使用各向异性采样
    //sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;

    //D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_WRAP;
    //D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_MIRROR;
    D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_CLAMP;
    //D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_BORDER;
    //D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    sampDesc.AddressU = address;
    sampDesc.AddressV = address;
    sampDesc.AddressW = address;
    sampDesc.BorderColor[0] = 0.0f;
    sampDesc.BorderColor[1] = 0.0f;
    sampDesc.BorderColor[2] = 0.0f;
    sampDesc.BorderColor[3] = 0.0f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));



    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 设置图元类型，设定输入布局
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());

    // PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    //设置纹理着色器资源
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
    m_pd3dImmediateContext->PSSetShaderResources(1, 1, m_pWoodCrate.GetAddressOf());
    //设置采样器
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosTexLayout");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Basic_3D_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Basic_3D_PS");
    D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");

    return true;
}


template <typename VertexType>
bool GameApp::ResetMesh(const Geometry::MeshData<VertexType>& meshData)
{
    // 释放旧资源
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(VertexType);	// 跨越字节数
    UINT offset = 0;							// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);



    // 设置索引缓冲区描述
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = meshData.indexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    // 输入装配阶段的索引缓冲区设置
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);



    // 设置调试对象名
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

    return true;
}
