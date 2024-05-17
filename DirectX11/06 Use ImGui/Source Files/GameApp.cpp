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
    static bool showImGuiWindow = true;
    static bool showDemoWindow = false;
    static bool showAnotherWindow = false;
    static bool animateCube = true; //物体运动
    static bool customColor = false;    //定制颜色

    static int dx = 0, dy = 0, dc = 0, dphi = 0;

    static float phi1 = 0.0f, theta1 = 0.0f;
    static float phi2 = 0.0f, theta2 = 0.0f;
    static float scale1 = 1.0f;
    static float scale2 = 1.0f;
    static float x1 = 0.0f, y1 = 0.0f;
    static float x2 = 0.0f, y2 = 0.0f;
    static float fov = XM_PIDIV2;

    //获取IO
    ImGuiIO& io = ImGui::GetIO();

    if (showImGuiWindow) {
        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }

        if (showAnotherWindow) {
            ImGui::Begin("Another Window");

            ImGui::Text("Another Window");

            ImGui::End();
        }

        if (animateCube) {
            phi1 += 0.3f * dt;
            theta1 += 0.3f * dt;
            //XMScalarModAngle(float Angle) 将传入的角限制在一定范围内
            phi1 = XMScalarModAngle(phi1);
            theta1 = XMScalarModAngle(theta1);

            phi2 += 0.3f * dt;
            theta2 += 0.3f * dt;
            //XMScalarModAngle(float Angle) 将传入的角限制在一定范围内
            phi2 = XMScalarModAngle(phi2);
            theta2 = XMScalarModAngle(theta2);
        }

        //自定义窗口
        if (ImGui::Begin("ImGui")) {

            ImGui::Text("Use of ImGui");

            ImGui::Checkbox("Demo Window", &showDemoWindow);
            ImGui::Checkbox("Another Window", &showAnotherWindow);
            ImGui::Checkbox("Animate Cube", &animateCube);

            if (ImGui::Button("Reset Params")) {
                x1 = y1 = phi1 = theta1 = 0.0f;
                scale1 = 1.0f;
                x2 = y2 = phi2 = theta2 = 0.0f;
                scale2 = 1.0f;
                fov = XM_PIDIV2;
            }

            ImGui::Text("Scale: %0.3f", scale1);
            ImGui::SliderFloat("Scale", &scale1, 0.2f, 2.0f);

            ImGui::Text("Phi: %0.3f", XMConvertToDegrees(phi1));
            ImGui::SliderFloat("X Axis Rotation", &phi1, -XM_PI, XM_PI);

            ImGui::Text("Theta: %0.3f", XMConvertToDegrees(theta1));
            ImGui::SliderFloat("Y Axis Rotation", &theta1, -XM_PI, XM_PI);

            ImGui::Text("Fov: %0.3f", XMConvertToDegrees(fov));
            ImGui::SliderFloat("Fov", &fov, XM_PIDIV4, XM_PI / 3 * 2);

            if (!ImGui::Checkbox("Use Custom Color", &customColor)) {
                m_CBuffer.useCustomColor = customColor;
            }
            if (customColor) {
                ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&m_CBuffer.color));
            }

            ImGui::Text("Model Position: (%.1f, %.1f, 0.0)", x1, y1);

            ImGui::End();
        }
    }

    //键鼠操作
    if (!ImGui::IsAnyItemActive()) {    //操作UI时不对物体进行影响
        // 鼠标左键拖动平移
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            x1 += io.MouseDelta.x * 0.01f;
            y1 -= io.MouseDelta.y * 0.01f;
        }
        // 鼠标右键拖动旋转
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            phi1 -= io.MouseDelta.y * 0.01f;
            theta1 -= io.MouseDelta.x * 0.01f;
            phi1 = XMScalarModAngle(phi1);
            theta1 = XMScalarModAngle(theta1);
        }
        // 鼠标滚轮缩放
        else if (io.MouseWheel != 0.0f) {
            scale1 += 0.02f * io.MouseWheel;
            if (scale1 > 2.0f)
                scale1 = 2.0f;
            else if (scale1 < 0.2f)
                scale1 = 0.2f;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {  //W键向上移动
            dy = 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_S)) {  //S键向下移动  
            dy = -1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_D)) {  //D键向右移动
            dx = 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_A)) {  //A键向左移动
            dx = -1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            dc = 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Q)) {
            dc = -1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_C)) {
            dphi = 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Z)) {
            dphi = -1;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_C) || ImGui::IsKeyReleased(ImGuiKey_Z)) {  //松开停止
            dphi = 0;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_E) || ImGui::IsKeyReleased(ImGuiKey_Q)) {  //松开停止
            dc = 0;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_W) || ImGui::IsKeyReleased(ImGuiKey_S)) {  //松开停止
            dy = 0;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_A) || ImGui::IsKeyReleased(ImGuiKey_D)) {  //松开停止
            dx = 0;
        }
    }

    x2 += dx * 0.0002f;  //更新位置
    y2 += dy * 0.0002f;  //更新位置

    scale2 += dc * 0.0002f;
    if (scale2 > 2.0f) {
        scale2 = 2.0f;
    }
    else if (scale2 < 0.2f) {
        scale2 = 0.2f;
    }

    phi2 -= dphi * 0.0002f;
    theta2 -= dphi * 0.0002f;
    phi2 = XMScalarModAngle(phi2);
    theta2 = XMScalarModAngle(theta2);




    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float pink[4] = { 255.0f / 255.0f,192.0f / 255.0f, 203.0f / 255.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&pink));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    //更改常量缓冲区
    //XMVECTOR XM_CALLCONV XMVectorReplicate(float Value);  返回向量(Value, Value, Value, Value)
    m_CBuffer.world = XMMatrixTranspose(
        XMMatrixScalingFromVector(XMVectorReplicate(scale1)) *
        XMMatrixRotationX(phi1) * XMMatrixRotationY(theta1) *
        XMMatrixTranslation(x1, y1, 0.0f)
    );
    //XMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)   透视投影矩阵
    //FovAngleY:视场角 AspectRatio:宽高比 
    m_CBuffer.proj = XMMatrixTranspose((XMMatrixPerspectiveFovLH(fov, AspectRatio(), 1.0f, 1000.0f)));

    //更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData1;
    m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData1);
    memcpy_s(mappedData1.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);


    //在输入装配阶段绑定顶点缓冲区
    UINT strides = sizeof(VertexPosColor);  //跨越字节数
    UINT offset = 0;    //起始偏移量
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer1.GetAddressOf(), &strides, &offset);

    //绑定索引缓冲区
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndicesBuffer1.Get(), DXGI_FORMAT_R32_UINT, 0);

    //绑定常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
    //这次需绑定到像素缓冲区，不然自定义的颜色无法使用
    m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    m_pd3dImmediateContext->DrawIndexed(18, 0, 0);







    //更改常量缓冲区
//XMVECTOR XM_CALLCONV XMVectorReplicate(float Value);  返回向量(Value, Value, Value, Value)
    m_CBuffer.world = XMMatrixTranspose(
        XMMatrixScalingFromVector(XMVectorReplicate(scale2)) *
        XMMatrixRotationX(phi2) * XMMatrixRotationY(theta2) *
        XMMatrixTranslation(x2, y2, 0.0f)
    );
    //XMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)   透视投影矩阵
    //FovAngleY:视场角 AspectRatio:宽高比 
    m_CBuffer.proj = XMMatrixTranspose((XMMatrixPerspectiveFovLH(fov, AspectRatio(), 1.0f, 1000.0f)));

    //更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData2;
    m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData2);
    memcpy_s(mappedData2.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer2.GetAddressOf(), &strides, &offset);

    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndicesBuffer2.Get(), DXGI_FORMAT_R32_UINT, 0);

    m_pd3dImmediateContext->DrawIndexed(36, 0, 0);

    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));

}

void GameApp::DrawScene()
{
}

//用于初始化着色器或相关特效
bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    //读取编译好的二进制文件
    HR(D3DReadFileToBlob(L"HLSL\\Cube_VS.cso", blob.GetAddressOf()));
    //创建顶点着色器
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    //创建顶点输入布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));


    //读取编译好的二进制文件
    HR(D3DReadFileToBlob(L"HLSL\\Cube_PS.cso", blob.GetAddressOf()));
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
        {XMFLOAT3(-3.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f)},
        {XMFLOAT3(-3.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-2.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)},
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
    VertexPosColor vertices1[] = {
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(3.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(3.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(3.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(3.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
    };

    //VertexPosColor vertices[] =
    //{
    //     { XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    //     { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    //     { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    //};

    //创建缓冲区

    //创建顶点缓冲区描述
    D3D11_BUFFER_DESC vbd1;
    ZeroMemory(&vbd1, sizeof(vbd1));  //将缓冲区数据置零
    vbd1.ByteWidth = sizeof(vertices2);
    vbd1.Usage = D3D11_USAGE_IMMUTABLE;  //设置gpu只读权限
    vbd1.BindFlags = D3D11_BIND_VERTEX_BUFFER;   //设置标志为顶点缓冲区
    vbd1.CPUAccessFlags = 0;

    //创建顶点子资源
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices2;

    //创建顶点缓冲区
    HR(m_pd3dDevice->CreateBuffer(&vbd1, &InitData, m_pVertexBuffer1.GetAddressOf()));

    //创建顶点缓冲区描述
    D3D11_BUFFER_DESC vbd2;
    ZeroMemory(&vbd2, sizeof(vbd2));  //将缓冲区数据置零
    vbd2.ByteWidth = sizeof(vertices1);
    vbd2.Usage = D3D11_USAGE_IMMUTABLE;  //设置gpu只读权限
    vbd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;   //设置标志为顶点缓冲区
    vbd2.CPUAccessFlags = 0;

    InitData.pSysMem = vertices1;
    HR(m_pd3dDevice->CreateBuffer(&vbd2, &InitData, m_pVertexBuffer2.GetAddressOf()));




    //使用索引，减少重复的点，索引数组
    DWORD indices1[] = {
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

    //设置四棱锥索引
    DWORD indices2[] = {
        0,2,3,
        0,1,2,
        0,4,1,
        1,4,2,
        3,4,0,
        2,4,3,
    };

    //设置索引缓冲区的描述
    D3D11_BUFFER_DESC ibd1;
    ZeroMemory(&ibd1, sizeof(ibd1));
    ibd1.Usage = D3D11_USAGE_IMMUTABLE;
    ibd1.ByteWidth = sizeof(indices2);
    ibd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd1.CPUAccessFlags = 0;

    //建立索引缓冲区
    InitData.pSysMem = indices2;
    HR(m_pd3dDevice->CreateBuffer(&ibd1, &InitData, m_pIndicesBuffer1.GetAddressOf()));

    D3D11_BUFFER_DESC ibd2;
    ZeroMemory(&ibd2, sizeof(ibd2));
    ibd2.Usage = D3D11_USAGE_IMMUTABLE;
    ibd2.ByteWidth = sizeof(indices1);
    ibd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd2.CPUAccessFlags = 0;

    InitData.pSysMem = indices1;
    HR(m_pd3dDevice->CreateBuffer(&ibd2, &InitData, m_pIndicesBuffer2.GetAddressOf()));




    //常量缓冲区描述
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    //建立常量缓冲区
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));


    //初始化常量缓冲区
    m_CBuffer.world = XMMatrixIdentity();    // 单位矩阵的转置是它本身
    m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));

    //使用透视投影
    //XMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)   透视投影矩阵
    //FovAngleY:视场角     AspectRatio:宽高比 
    m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

    m_CBuffer.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_CBuffer.useCustomColor = false;



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
    D3D11SetDebugObjectName(m_pVertexBuffer1.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndicesBuffer1.Get(), "IndicesBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

    return true;
}