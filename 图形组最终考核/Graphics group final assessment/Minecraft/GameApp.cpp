#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight), m_StoreChunkNum((int)pow(DSM::Chunk::m_StoreChunkRadius * 2, 2)){}

GameApp::~GameApp() {}

bool GameApp::Init()
{
    PROFILE_FUNCTION();

    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_PostProcessEffect.InitAll(m_pd3dDevice.Get())) {
        return false;
    }

    if (!m_RainEffect.InitAll(m_pd3dDevice.Get(), L"Shaders\\Rain.hlsl"))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();

    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");
    m_pLitTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pLitTexture->SetDebugObjectName("LitTexture");

    // 摄像机变更显示
    if (m_pTCamera != nullptr && m_pFCamera != nullptr){
        m_pFCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pFCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
        m_SkyboxEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
        m_RainEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
        m_RainEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{
    PROFILE_FUNCTION();

    if (m_FadeUsed) {
        m_FadeCount += m_FadeSign * dt / 2.0f;	// 2s时间淡入/淡出
        if (m_FadeSign > 0.0f && m_FadeCount > 1.0f) {  //开机淡入
            m_FadeCount = 1.0f;
            m_FadeUsed = false;	// 结束淡入
        }
        else if (m_FadeSign < 0.0f && m_FadeCount < 0.0f) { // 关机淡出
            m_FadeCount = 0.0f;
            SendMessage(MainWnd(), WM_DESTROY, 0, 0);	// 关闭程序
            // 这里不结束淡出是因为发送关闭窗口的消息还要过一会才真正关闭
        }
    }


    XMFLOAT3 cameraPos = m_pFCamera->GetPosition();

    // 获取人物可能触及的物块
    DSM::Chunk* inChunk = nullptr;
    std::vector<BoundingBox>* containBlock;
    for (auto& chunk : m_Chunk) {
        XMINT2 chunkPosition = chunk.GetPositon();
        if (chunkPosition.x <= cameraPos.x && cameraPos.x < chunkPosition.x + CHUNKSIZE &&
            chunkPosition.y <= cameraPos.z && cameraPos.z < chunkPosition.y + CHUNKSIZE) {
            inChunk = &chunk;
            containBlock = &chunk.GetBlockBox();
            //containBlock = chunk.GetBlockId();
            break;
        }
    }

    if (!inChunk) { // 若超出界限
        int cameraPosX = (int)cameraPos.x;
        int cameraPosZ = (int)cameraPos.z;
        if (cameraPosX < 0) {
            cameraPosX -= 16;
        }
        if (cameraPosZ < 0) {
            cameraPosZ -= 16;
        }
        XMINT2 chunkPos(cameraPosX - cameraPosX % CHUNKSIZE, cameraPosZ - cameraPosZ % CHUNKSIZE);  // 获取应该加载的区块的坐标

        LoadChunk(chunkPos);    // 禁止移动操作并加载当前区块
    }
    else {
        XMINT2 inChunkPos = inChunk->GetPositon();
        LoadChunk(inChunkPos);
        PlaceDestroyBlocks(inChunk);
        CameraTransform(dt, *containBlock);
        //CameraTransform(dt, containBlock);CameraTransform(dt, containBlock);
    }


    if (m_EnableRain) {
        ParticleSystem(dt);
    }

    EnemyManagement();

    DayAndNightChange(dt);

    ImGuiOperations(dt);    // ImGui操作
}

void GameApp::DrawScene()
{
    PROFILE_FUNCTION();

    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount) {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    DrawScene(m_FadeUsed ? m_pLitTexture->GetRenderTarget() : GetBackBufferRTV(),
        m_pDepthTexture->GetDepthStencil(),
         m_pFCamera->GetViewPort());


    // 绘制小地图到场景
    CD3D11_VIEWPORT minimapViewport(
        std::max(0.0f, m_ClientWidth - 300.0f),
        std::max(0.0f, 0.0f),
        std::min(300.0f, (float)m_ClientWidth),
        std::min(300.0f, (float)m_ClientHeight));
    m_PostProcessEffect.RenderMinimap(
        m_pd3dImmediateContext.Get(),
        m_pMinimapTexture->GetShaderResource(),
        m_FadeUsed ? m_pLitTexture->GetRenderTarget() : GetBackBufferRTV(),
        minimapViewport);

    if (m_FadeUsed) {   // 绘制渐变过程
        m_PostProcessEffect.RenderScreenFade(m_pd3dImmediateContext.Get(), m_pLitTexture->GetShaderResource(), GetBackBufferRTV(), m_pFCamera->GetViewPort(), m_FadeCount);
    }

    // 保存到output.dds和output.png中
    if (m_PrintScreenStarted) {
        ComPtr<ID3D11Texture2D> backBuffer;
        GetBackBufferRTV()->GetResource(reinterpret_cast<ID3D11Resource**>(backBuffer.GetAddressOf()));
        // 输出截屏
        HR(SaveWICTextureToFile(m_pd3dImmediateContext.Get(), backBuffer.Get(), GUID_ContainerFormatJpeg, L"Output.jpg", &GUID_WICPixelFormat24bppBGR));
        // 结束截屏
        m_PrintScreenStarted = false;
    }


    ID3D11RenderTargetView* pRTVs[] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}







static std::mutex s_ChunkMutex;

// 多线程并行函数
static void ParallelLoadChunk(std::vector<DSM::Chunk>& m_Chunk, DSM::Chunk chunk, TextureManager& tManager, ModelManager& mManager)
{
    chunk.LoadChunk();
    std::lock_guard<std::mutex> lock(s_ChunkMutex);
    m_Chunk.push_back(chunk);
}


bool GameApp::InitResource()
{
    PROFILE_FUNCTION();

    InitSkybox();
    InitCamara();
    InitRain();

    m_Player.SetModel(m_pFCamera, m_ModelManager);


    int loadRadius = (int)sqrt(m_StoreChunkNum) / 2;
    
    // 加载区块
    m_Chunk.reserve(m_StoreChunkNum);

#define ASYNC 0
#if ASYNC
    DSM::Chunk initialChunk(0, 0);
    initialChunk.InitBlock(m_TextureManager, m_ModelManager);
    initialChunk.LoadChunk();
    m_Chunk.push_back(std::move(initialChunk));
    for (int i = -loadRadius; i < loadRadius; ++i) {
        for (int j = -loadRadius; j < loadRadius; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            DSM::Chunk chunk(0 + CHUNKSIZE * i, 0 + CHUNKSIZE * j);
            chunk.InitBlock(m_TextureManager, m_ModelManager);
            m_Futures.push_back(std::async(std::launch::async, ParallelLoadChunk, std::ref(m_Chunk), chunk, std::ref(m_TextureManager), std::ref(m_ModelManager)));
        }
    }

#else
    for (int i = -loadRadius; i < loadRadius; ++i) {
        for (int j = -loadRadius; j < loadRadius; ++j) {
            DSM::Chunk& chunk = m_Chunk.emplace_back(0 + CHUNKSIZE * i, 0 + CHUNKSIZE * j);
            chunk.InitBlock(m_TextureManager, m_ModelManager);
            chunk.LoadChunk();
        }
    }

#endif

    XMINT4 treeRange(-m_ViewRange * CHUNKSIZE, m_ViewRange * CHUNKSIZE, -m_ViewRange * CHUNKSIZE, m_ViewRange * CHUNKSIZE);
    m_CherryTree.CreateRandomTree(treeRange, m_ModelManager, m_TextureManager);

    GameObject object;
    object.SetModel(nullptr);
    m_Dirt.resize(64, DSM::Block(object, DSM::BlockId::Dirt));
    

    // ******************
    // 初始化光栅化状态
    //  

    // 方向光
    m_DirLight[0].ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    m_DirLight[0].diffuse = XMFLOAT4(m_Diffuse, m_Diffuse, m_Diffuse, 1.0f);
    m_DirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    m_DirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    m_DirLight[1] = m_DirLight[0];
    m_DirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
    m_DirLight[2] = m_DirLight[0];
    m_DirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
    m_DirLight[3] = m_DirLight[0];
    m_DirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, m_DirLight[i]);
    
    m_CherryTree.m_EnableTreeFC = false;
    m_EnableChunkFrustumCulling = false;
    // 要最后初始化小地图
    InitMiniMap();
    m_CherryTree.m_EnableTreeFC = true;
    m_EnableChunkFrustumCulling = true;

    m_FogEnabled = true;
    m_BasicEffect.SetFogState(m_FogEnabled);
    m_BasicEffect.SetFogStart(m_FogStart);
    m_BasicEffect.SetFogRange(m_FogRange);
    m_BasicEffect.SetFogColor(XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f));

    return true;
}

void GameApp::InitRain()
{
    PROFILE_FUNCTION();

    m_RainEffect.SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
    m_RainEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
    m_RainEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
    // ******************
    // 初始化粒子系统
    //
    m_TextureManager.CreateFromFile("..\\Texture\\rain\\raindrop.dds", false, true);

    // 创建随机数据
    std::mt19937 randEngine;
    randEngine.seed(std::random_device()());
    std::uniform_real_distribution<float> randF(-1.0f, 1.0f);
    std::vector<float> randomValues(4096);

    // 生成1D随机纹理
    CD3D11_TEXTURE1D_DESC texDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, 1024, 1, 1);
    D3D11_SUBRESOURCE_DATA initData{ randomValues.data(), 1024 * GetFormatSize(DXGI_FORMAT_R32G32B32A32_FLOAT) };
    ComPtr<ID3D11Texture1D> pRandomTex;
    ComPtr<ID3D11ShaderResourceView> pRandomTexSRV;

    std::generate(randomValues.begin(), randomValues.end(), [&]() { return randF(randEngine); });
    HR(m_pd3dDevice->CreateTexture1D(&texDesc, &initData, pRandomTex.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateShaderResourceView(pRandomTex.Get(), nullptr, pRandomTexSRV.ReleaseAndGetAddressOf()));
    m_TextureManager.AddTexture("RainRandomTex", pRandomTexSRV.Get());

    m_Rain.InitResource(m_pd3dDevice.Get(), 10000);
    m_Rain.SetTextureInput(m_TextureManager.GetTexture("..\\Texture\\rain\\raindrop.dds"));
    m_Rain.SetTextureRandom(m_TextureManager.GetTexture("RainRandomTex"));
    m_Rain.SetEmitDir(XMFLOAT3(0.0f, -1.0f, 0.0f));
    m_Rain.SetAcceleration(XMFLOAT3(-1.0f, -9.8f, 0.0f));
    m_Rain.SetEmitInterval(0.0015f);
    m_Rain.SetAliveTime(3.0f);
    m_Rain.SetDebugObjectName("Rain");
}

void GameApp::InitCamara()
{
    PROFILE_FUNCTION();

    // ******************
    // 初始化摄像机
    XMFLOAT3 position(0.0f, SEALEVEL + (int)(DSM::Chunk::GetNoice(0, 0)) + 2.3f, 0.0f);
    auto fCamera = std::make_shared<FirstPersonCamera>();
    auto tCamera = std::make_shared<ThirdPersonCamera>();
    m_pFCamera = fCamera;
    m_pTCamera = tCamera;
    m_FCameraControl.InitCamera(fCamera.get());
    m_FPCameraControl.InitCamera(fCamera.get());
    m_TCameraControl.InitCamera(tCamera.get());
    fCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    fCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.1f, 1000.0f);
    fCamera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
    fCamera->SetPosition(position);
    tCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    tCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.1f, 1000.0f);
    tCamera->SetTarget(position);
    tCamera->SetDistanceMinMax(2.0f, 6.0f);
    tCamera->SetDistance(3.5f);

    m_BasicEffect.SetViewMatrix(fCamera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(fCamera->GetProjMatrixXM());
    m_SkyboxEffect.SetViewMatrix(fCamera->GetViewMatrixXM());
    m_SkyboxEffect.SetProjMatrix(fCamera->GetProjMatrixXM());
}

void GameApp::InitSkybox()
{
    PROFILE_FUNCTION();

    // 初始化天空盒
    ComPtr<ID3D11Texture2D> pTex;
    D3D11_TEXTURE2D_DESC texDesc;
    std::string filenameStr;
    std::vector<ID3D11ShaderResourceView*> pCubeTextures;
    std::unique_ptr<TextureCube> pTexCube;
    filenameStr = "..\\Texture\\skybox\\daylight0.png";
    for (size_t i = 0; i < 6; ++i)
    {
        filenameStr[26] = '0' + (char)i;
        pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
    }
    pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
    pTex->GetDesc(&texDesc);
    pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    pTexCube->SetDebugObjectName("Daylight");
    for (uint32_t i = 0; i < 6; ++i)
    {
        pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
    }
    m_TextureManager.AddTexture("Daylight", pTexCube->GetShaderResource());
    // 天空盒立方体
    Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
    pModel->SetDebugObjectName("Skybox");
    pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
    m_Skybox.SetModel(pModel);
}

void GameApp::InitMiniMap()
{
    PROFILE_FUNCTION();

    // 初始化小地图
    m_pMinimapTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), 300.0f, 300.0f, DXGI_FORMAT_R8G8B8A8_UNORM);
    std::unique_ptr<Depth2D> pMinimapDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), 300.0f, 300.0f);
    m_pMinimapTexture->SetDebugObjectName("MinimapTexture");
    pMinimapDepthTexture->SetDebugObjectName("MinimapDepthTexture");
    CD3D11_VIEWPORT minimapViewport(0.0f, 0.0f, 300.0f, 300.0f);

    // 小地图可视范围
    m_PostProcessEffect.SetMinimapRect(XMFLOAT4(-100.0f, 100.0f, 100.0f, -100.0f));
    m_PostProcessEffect.SetVisibleRange(35.0f);

    // 渲染小地图纹理
    m_BasicEffect.SetViewMatrix(XMMatrixLookToLH(
        XMVectorSet(0.0f, SEALEVEL + 20.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)));
    // 使用正交投影矩阵(中心在摄像机位置)
    m_BasicEffect.SetProjMatrix(XMMatrixOrthographicLH(190.0f, 190.0f, 1.0f, SEALEVEL + 20.0f));
    DrawScene(m_pMinimapTexture->GetRenderTarget(), pMinimapDepthTexture->GetDepthStencil(), minimapViewport);

    m_BasicEffect.SetProjMatrix(m_pFCamera->GetProjMatrixXM());
}



// 放置与破坏方块
void GameApp::PlaceDestroyBlocks(DSM::Chunk* inChunk)
{
    std::vector<Transform>& dirtTransform = inChunk->GetDirtTransform();
    std::vector<Transform>& bedRockTransform = inChunk->GetBedRockTransform();
    std::vector<Transform>& stoneTransform = inChunk->GetStoneTransform();
    std::vector<Transform>& gressTransform = inChunk->GetGressTransform();
    std::vector<BasicEffect::InstancedData>& dirtData = inChunk->GetDirtInstancedData();
    std::vector<BasicEffect::InstancedData>& bedRockData = inChunk->GetBedRockInstancedData();
    std::vector<BasicEffect::InstancedData>& stoneData = inChunk->GetStoneInstancedData();
    std::vector<BasicEffect::InstancedData>& gressData = inChunk->GetGressInstancedData();

    std::vector<BoundingBox>& blockBox = inChunk->GetBlockBox();

    //获取鼠标
    ImVec2 mousePos = ImGui::GetMousePos();
    // 将射线设置在鼠标处
    Ray ray = Ray::ScreenToRay(*m_pFCamera, mousePos.x, mousePos.y);

    static size_t create = 0;
    for (size_t i = 0; i < m_SoilNum && m_Dirt[create].GetBlock().GetModel(); i++) {
        if (!m_Dirt[i].GetBlock().GetModel()) {
            create = i;
            break;
        }
    }
    //放置方块
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
#if 0
        bool Placing = true;
        GameObject tmpObject;
        tmpObject.SetModel(DSM::BlockModel().GetDirtModel(m_TextureManager.Get(), m_ModelManager.Get()));
        DirectX::XMFLOAT3 position = m_pFCamera->GetPosition();
        DirectX::XMFLOAT3 lookAxis = m_pFCamera->GetLookAxis();
        tmpObject.GetTransform().SetPosition(position.x + lookAxis.x * 4, position.y + lookAxis.y * 4, position.z + lookAxis.z * 4);
        for (size_t i = 0; i < m_Dirt.size(); i++) {
            if (m_Dirt[i].GetBlock().GetModel() && m_Dirt[i].GetBlock().GetBoundingBox().Intersects(tmpObject.GetBoundingBox())) {
                Placing = false;
                break;
            }
        }
        if (Placing) {
            m_Dirt[create] = (DSM::Block(tmpObject, DSM::BlockId::Dirt));
            create++;
        }
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        for (size_t i = 0; i < m_SoilNum; i++) {
            if (ray.Hit(m_Dirt[i].GetBlock().GetBoundingBox(), nullptr, RAYRANGE)) {
                m_Dirt[i].GetBlock().SetModel(nullptr);
                create = i;
            }
        }
#else
        // 射线打中的方块
        std::vector<BoundingBox> hitBlock;
        XMFLOAT3 extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
        GameObject tmpObject;
        tmpObject.SetModel(DSM::BlockModel().GetDirtModel(m_TextureManager.Get(), m_ModelManager.Get()));
        DirectX::XMFLOAT3 position = m_pFCamera->GetPosition();
        // 筛选射线打中的方块
        for (int Y = (int)position.y - RAYRANGE; 0 <= Y && Y < CHUNKHIGHEST && Y < (int)position.y + RAYRANGE; ++Y) {
            for (int z = 0; z < CHUNKSIZE; ++z) {
                for (int x = 0; x < CHUNKSIZE; ++x) {
                    if (ray.Hit(blockBox[Y * CHUNKSIZE * CHUNKSIZE + z * CHUNKSIZE + x], nullptr, RAYRANGE)) {
                        hitBlock.emplace_back(XMFLOAT3(x + 0.5f, Y + 0.5f, z + 0.5f), extents);
                    }
                }
            }
        }
        // 选取最近的方块
        float distance;
        int pos = 0;
        for (int i = 0; i < hitBlock.size(); ++i) {
            XMVECTOR vDistance = XMVectorSubtract(XMLoadFloat3(&hitBlock[i].Center), XMLoadFloat3(&position));
            vDistance = XMVectorMultiply(vDistance, vDistance);
            float fDistance = (XMVectorGetX(vDistance) + XMVectorGetY(vDistance) + XMVectorGetZ(vDistance));
            if (i == 0) {
                distance = fDistance;
            }
            else if (distance > fDistance) {
                distance = fDistance;
                pos = i;
            }
        }
        if (hitBlock.size() > 0) {
            XMFLOAT3 boxPosition = hitBlock[pos].Center;
            XMFLOAT3 newPosition = XMFLOAT3(boxPosition.x - 0.5f, boxPosition.y + 0.5f, boxPosition.z - 0.5f);
            tmpObject.GetTransform().SetPosition(newPosition);
            BasicEffect::InstancedData instanceData;
            XMMATRIX W(tmpObject.GetTransform().GetLocalToWorldMatrixXM());
            XMStoreFloat4x4(&instanceData.world, XMMatrixTranspose(W));
            XMStoreFloat4x4(&instanceData.worldInvTranspose, XMMatrixTranspose(XMath::InverseTranspose(W)));
            dirtData.push_back(instanceData);
            blockBox[newPosition.y * CHUNKSIZE * CHUNKSIZE + newPosition.z * CHUNKSIZE + newPosition.x] =
                BoundingBox(XMFLOAT3(newPosition.x + 0.5f, newPosition.y + 0.5f, newPosition.z + 0.5f), extents);
        }
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        for (size_t i = 0; i < m_SoilNum; i++) {
            if (ray.Hit(m_Dirt[i].GetBlock().GetBoundingBox(), nullptr, RAYRANGE)) {
                m_Dirt[i].GetBlock().SetModel(nullptr);
                create = i;
            }
        }
#endif
    }
}



// 相机变换
void GameApp::CameraTransform(float dt, std::vector<BoundingBox>& containBlock)
{
    PROFILE_FUNCTION();

    // 获取玩家变换
    Transform& playerTransform = m_Player.GetEntity().GetTransform();
    XMFLOAT3 FCPosition = m_pFCamera->GetPosition();
    
    if (!(m_CameraMode == CameraMode::Free)) {
        m_TCameraControl.Update(dt);
        m_FPCameraControl.Update(dt, containBlock);
        XMFLOAT3 fPosition = m_pFCamera->GetPosition();
        playerTransform.SetPosition(fPosition.x, fPosition.y - 1.8f, fPosition.z);
        if (m_CameraMode == CameraMode::ThirdPerson) {
            playerTransform.SetRotation(0.0f, m_pTCamera->GetRotationY() - XM_PI, 0.0f);
            m_pTCamera->SetTarget(m_pFCamera->GetPosition());
            // 更新观察矩阵
            m_BasicEffect.SetViewMatrix(m_pTCamera->GetViewMatrixXM());
            m_BasicEffect.SetEyePos(m_pTCamera->GetPosition());
            m_SkyboxEffect.SetViewMatrix(m_pTCamera->GetViewMatrixXM());
            m_PostProcessEffect.SetEyePos(m_pTCamera->GetPosition());
        }
        else if (m_CameraMode == CameraMode::FirstPerson) {
            playerTransform.SetRotation(0.0f, m_pFCamera->GetRotationY() - XM_PI, 0.0f);
            // 更新观察矩阵
            m_BasicEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
            m_BasicEffect.SetEyePos(m_pFCamera->GetPosition());
            m_SkyboxEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
            m_PostProcessEffect.SetEyePos(m_pFCamera->GetPosition());
        }
    }
    else {
        m_FCameraControl.Update(dt);
        // 更新观察矩阵
        m_BasicEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
        m_BasicEffect.SetEyePos(m_pFCamera->GetPosition());
        m_SkyboxEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
        m_PostProcessEffect.SetEyePos(m_pFCamera->GetPosition());
    }
}



// ImGui操作
void GameApp::ImGuiOperations(float dt)
{
    PROFILE_FUNCTION();

    static float thirdDistance = 3.5f;
    static int cameraMode = 0;
    static const char* cModes[] = {
        "Free Camera",
        "First Person",
        "Third Person"
    };
    if (ImGui::Begin("Minecraft")) {
        if (ImGui::Button("Exit")||ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_FadeUsed = true;  //开始淡出
            m_FadeSign = -1.0f;
        }
        if (ImGui::Button("Save As Output.jpg")) {
            m_PrintScreenStarted = true;
        }
        ImGui::Checkbox("Enable Chunk Frustum Culling", &m_EnableChunkFrustumCulling);
        ImGui::Checkbox("Enable Block Frustum Culling", &DSM::Chunk::m_EnableFrustumCulling);
        ImGui::Checkbox("Enable Tree Frustum Culling", &DSM::CherryTree::m_EnableTreeFC);
        ImGui::Checkbox("Enable Rain", &m_EnableRain);
        ImGui::Checkbox("Enemy tracking", &m_EnemyTrack);
        static int fogMode = 0;
        static const char* fModes[] = {
            "Auto",
            "Daytime",
            "Dark Night",
        };
        if (ImGui::Checkbox("Enable Fog", &m_FogEnabled)) {
            m_BasicEffect.SetFogState(m_FogEnabled);
        }
        if (m_FogEnabled) {
            ImGui::Combo("Fog Mode", &fogMode, fModes, ARRAYSIZE(fModes));
            if (fogMode == 2) { // 黑夜模式下变为逐渐黑暗
                    m_IsNight = true;
                    m_BasicEffect.SetFogColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                    m_BasicEffect.SetFogStart(5.0f);
            }
            else if (fogMode == 1) {  // 白天模式则对应雾效
                m_IsNight = false;
                m_BasicEffect.SetFogColor(XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f));
                m_BasicEffect.SetFogStart(15.0f);
            }
            else {
                if (m_Diffuse < 0.15f) {
                    m_IsNight = true;
                }
                else {
                    m_IsNight = false;
                }
                m_BasicEffect.SetFogColor(XMFLOAT4(m_Diffuse - 0.1f, m_Diffuse - 0.1f, m_Diffuse - 0.1f, 1.0f));
                m_BasicEffect.SetFogStart(10.0f);
            }
            if (ImGui::SliderFloat("Fog Range", &m_FogRange, 40.0f, 175.0f, "%.0f")) {
                m_BasicEffect.SetFogRange(m_FogRange);
            }
            float fog_start = m_IsNight ? 5.0f : 15.0f;
            ImGui::Text("Fog: %.0f-%.0f", fog_start, m_FogRange + fog_start);
        }

        ImGui::Combo("Camera Mode", &cameraMode, cModes, ARRAYSIZE(cModes));
        if (0 == cameraMode) {
            m_CameraMode = CameraMode::Free;
        }
        else if (1 == cameraMode) {
            m_CameraMode = CameraMode::FirstPerson;
        }
        else if (2 == cameraMode) {
            if (!(m_CameraMode == CameraMode::ThirdPerson)) {
                m_pTCamera->SetRotationY(m_pFCamera->GetRotationY());
            }
            m_CameraMode = CameraMode::ThirdPerson;
        }
        ImGui::SliderFloat("Speed", &m_Player.GetSpeed(), 0.5f, 4.0f);
        ImGui::SliderFloat("Third Person Distance", &thirdDistance, 2.0f, 6.0f);

        ImGui::Text("X: %f",m_pFCamera->GetPosition().x);
        ImGui::Text("Y: %f", m_pFCamera->GetPosition().y);
        ImGui::Text("Z: %f", m_pFCamera->GetPosition().z);

        ImGui::End();
    }
    ImGui::Render();

    m_pTCamera->SetDistance(thirdDistance);
    m_FCameraControl.SetMoveSpeed(m_Player.GetSpeed());
    m_TCameraControl.SetMoveSpeed(m_Player.GetSpeed());
    m_FPCameraControl.SetMoveSpeed(m_Player.GetSpeed());
}



// 绘制主要场景
void GameApp::DrawScene(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, const D3D11_VIEWPORT& viewport)
{
    PROFILE_FUNCTION();

    m_pd3dImmediateContext->ClearRenderTargetView(pRTV, reinterpret_cast<const float*>(&Colors::Pink));
    m_pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    m_BasicEffect.SetRenderDefault();
    if (m_EnableChunkFrustumCulling) {
        ChunkFrustumCull();
    }
    else {
        for (auto& chunk : m_Chunk) {
            XMINT2 chunkPosition = chunk.GetPositon();
            XMFLOAT3 cameraPosition = m_pFCamera->GetPosition();
            float xDistance = abs(cameraPosition.x - chunkPosition.x);
            float yDistance = abs(cameraPosition.z - chunkPosition.y);
            if (xDistance < m_ViewRange * CHUNKSIZE && yDistance < m_ViewRange * CHUNKSIZE) {
                chunk.DrawChunk(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(), m_BasicEffect, m_pFCamera);
            }
        }
    }

     m_Player.GetEntity().Draw(m_pd3dImmediateContext.Get(), m_BasicEffect.Get());

     for (auto& enemy : m_Enemy) {
         enemy.DrawEnemy(m_pd3dImmediateContext.Get(), m_BasicEffect.Get());
     }

     m_CherryTree.DrawTree(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(), m_BasicEffect, m_pFCamera);

    for (auto& dirt : m_Dirt) {
        if (dirt.GetBlock().GetModel()) {
            dirt.GetBlock().Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
        }
    }

    // 绘制天空盒
    m_SkyboxEffect.SetRenderDefault();
    m_SkyboxEffect.SetSkyCount(m_pd3dImmediateContext.Get(), m_SkyColor);
    m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);

    // ******************
    // 粒子系统留在最后绘制便于混合
    //
    if (m_EnableRain) {
        m_pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, m_pDepthTexture->GetDepthStencil());
        m_Rain.Draw(m_pd3dImmediateContext.Get(), m_RainEffect);
    }

    pRTV = nullptr;
    m_pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, nullptr);
}



// 昼夜更替
void GameApp::DayAndNightChange(float dt)
{
    // 漫反射光的昼夜更替
    m_Diffuse += m_DiffuseSign * 0.012f * dt;
    if (m_Diffuse > 0.5f) {
        m_DiffuseSign = -1.0f;
    }
    else if (m_Diffuse < 0.06f) {
        m_DiffuseSign = 1.0f;
    }
    m_DirLight[0].diffuse = XMFLOAT4(m_Diffuse, m_Diffuse, m_Diffuse, 1.0f);
    m_DirLight[1].diffuse = XMFLOAT4(m_Diffuse, m_Diffuse, m_Diffuse, 1.0f);
    m_DirLight[2].diffuse = XMFLOAT4(m_Diffuse, m_Diffuse, m_Diffuse, 1.0f);
    m_DirLight[3].diffuse = XMFLOAT4(m_Diffuse, m_Diffuse, m_Diffuse, 1.0f);
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, m_DirLight[i]);

    // 天空盒的昼夜更替
    if (m_SkyColor > 0.1) {
        m_SkyColor += m_SkySign * 0.06f * dt;
    }
    else if (m_SkyColor > 0.04) {
        m_SkyColor += m_SkySign * 0.01f * dt;
    }
    else {
        m_SkyColor += m_SkySign * 0.004f * dt;
    }
    if (m_SkyColor > 1.0f) {
        m_SkySign = -1.0f;
    }
    else if (m_SkyColor < -0.02f) {
        m_SkySign = 1.0f;
    }
}



void GameApp::EnemyManagement()
{
    ImVec2 mousePos = ImGui::GetMousePos();
    Ray ray = Ray::ScreenToRay(*m_pFCamera, mousePos.x, mousePos.y);

    // 加载敌人
    for (auto& enemy : m_Enemy) {
        if (m_EnemyTrack) {
            enemy.FindPlayer(m_Player.GetEntity().GetTransform().GetPosition());
        }
        enemy.LoadEnemy(m_TextureManager, m_ModelManager);
    }
    // 血量扣除
    for (std::vector<DSM::Enemy>::iterator it = m_Enemy.begin(); it != m_Enemy.end(); ++it) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ray.Hit((*it).GetEntity().GetBoundingBox(), nullptr, 6.0f)) {
            --(*it).GetHP();
            if ((*it).GetHP() <= 0) {
                m_Enemy.erase(it);
                break;
            }
        }
    }
    // 夜晚生成敌人
    if (m_IsNight && m_Enemy.size() < 10) {
        std::mt19937 random;	// 梅森旋转
        random.seed(std::random_device()());	// 设置种子
        for (int i = 0; i < 10; i++) {
            DSM::Enemy enemy;
            enemy.SetModel(m_TextureManager, m_ModelManager);
            std::uniform_real<float> randomDistX(-50.f, 50.f);
            std::uniform_real<float> randomDistZ(-50.f, 50.f);
            XMFLOAT3 playerPosition = m_Player.GetEntity().GetTransform().GetPosition();
            int X = (int)(playerPosition.x + randomDistX(random));
            int Z = (int)(playerPosition.y + randomDistZ(random));
            int Y = SEALEVEL + (int)(DSM::Chunk::GetNoice(X, Z) + 0.2f);
            enemy.SetPosition((float)X, (float)Y, (float)Z);
            m_Enemy.push_back(std::move(enemy));
        }
    }
}



void GameApp::ParticleSystem(float dt)
{
    // ******************
    // 粒子系统
    //
    m_Rain.Update(dt, m_Timer.TotalTime());


    m_RainEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
    m_RainEffect.SetEyePos(m_pFCamera->GetPosition());

    static XMFLOAT3 lastCameraPos = m_pFCamera->GetPosition();
    XMFLOAT3 cameraPos = m_pFCamera->GetPosition();

    XMVECTOR cameraPosVec = XMLoadFloat3(&cameraPos);
    XMVECTOR lastCameraPosVec = XMLoadFloat3(&lastCameraPos);
    XMFLOAT3 emitPos;
    XMStoreFloat3(&emitPos, cameraPosVec + 3.0f * (cameraPosVec - lastCameraPosVec));
    m_RainEffect.SetViewMatrix(m_pFCamera->GetViewMatrixXM());
    m_RainEffect.SetEyePos(m_pFCamera->GetPosition());
    m_Rain.SetEmitPos(emitPos);
    lastCameraPos = m_pFCamera->GetPosition();
}






struct XMINT2Less
{
    bool operator()(const XMINT2& left, const XMINT2& right) const
    {
        if (left.x == right.x) {
            return left.y < right.y;
        }
        return left.x < right.x;
    }
};


void GameApp::LoadChunk(const XMINT2& inChunkPos)
{
    PROFILE_FUNCTION();

    static int pos = m_Chunk.size();
    int viewRange = (int)pow(m_ViewRange * 2, 2);    // 可视区块个数
    std::set<XMINT2, XMINT2Less> chunkPosition;             // 需要加载的区块的位置
    std::vector<XMINT2> shouldLoad;                     // 需要加载的区块

    // 存储区块位置
    for (auto& chunk : m_Chunk) {
        chunkPosition.insert(chunk.GetPositon());
    }

    // 筛选视距内的区块
    for (int i = -m_ViewRange; i < m_ViewRange; ++i) {
        for (int j = -m_ViewRange; j < m_ViewRange; ++j) {
            XMINT2 position(inChunkPos.x + i * CHUNKSIZE, inChunkPos.y + j * CHUNKSIZE);
            // 插入成功则是新区块
            if (chunkPosition.insert(position).second) {
                shouldLoad.emplace_back(position);
                break;
            }
        }
    }

#if ASYNC
    for (; shouldLoad.size() > 0;) {
        DSM::Chunk chunk(shouldLoad.back());
        chunk.InitBlock(m_TextureManager, m_ModelManager);
        shouldLoad.pop_back();
        m_Futures.push_back(std::async(std::launch::async, ParallelLoadChunk, std::ref(m_Chunk), chunk, std::ref(m_TextureManager), std::ref(m_ModelManager)));
    }

#else
    // 每帧加载一个区块
    if (shouldLoad.size() > 0) {
        DSM::Chunk& chunk = m_Chunk.emplace_back(std::move(shouldLoad.front()));
        chunk.InitBlock(m_TextureManager, m_ModelManager);
        chunk.LoadChunk();
    }

#endif

    // 卸载区块
    for (std::vector<DSM::Chunk>::iterator it = m_Chunk.begin();
        m_Chunk.size() > pow(m_StoreChunkNum, 2) && it != m_Chunk.end();) {
        if ((*it).UnloadChunk(inChunkPos)) {    // 移除加载范围外的区块
            it = m_Chunk.erase(it);
        }
        else {
            ++it;
        }
    }
}



void GameApp::ChunkFrustumCull()
{
    PROFILE_FUNCTION();

    // 筛选要绘制的区块
    for (auto& chunk : m_Chunk) {
        XMINT2 chunkPosition = chunk.GetPositon();
        XMFLOAT3 cameraPosition = m_pFCamera->GetPosition();
        float xDistance = abs(cameraPosition.x - chunkPosition.x);
        float yDistance = abs(cameraPosition.z - chunkPosition.y);
        if (xDistance < m_ViewRange * CHUNKSIZE && yDistance < m_ViewRange * CHUNKSIZE) {
            // 对区块进行剔除
            BoundingFrustum frustum;
            BoundingFrustum::CreateFromMatrix(frustum, m_pFCamera->GetProjMatrixXM());  // 视锥体包围盒
            BoundingBox localBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(CHUNKSIZE / 2, CHUNKHIGHEST / 2, CHUNKSIZE / 2)), box;
            XMINT2 chunkPosition = chunk.GetPositon();
            Transform chunkTransform;
            chunkTransform.SetPosition((float)(chunkPosition.x + CHUNKSIZE / 2), (float)(CHUNKHIGHEST / 2), (float)(chunkPosition.y + CHUNKSIZE / 2));
            localBox.Transform(box, chunkTransform.GetLocalToWorldMatrixXM() * m_pFCamera->GetViewMatrixXM());
            if (frustum.Intersects(box)) {
                chunk.DrawChunk(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(), m_BasicEffect.Get(), m_pFCamera);
            }
        }
    }
}