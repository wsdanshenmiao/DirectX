#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>
#include <DirectXColors.h>
#include <wincodec.h> // 使用ScreenGrab11.h需要
#include <ScreenGrab11.h>
#include "Block.h"
#include "BlockModel.h"
#include "Chunk.h"
#include "Player.h"
#include "MyCameraControl.h"
#include "CherryTree.h"
#include "Enemy.h"

#define RAYRANGE 4

// 摄像机模式
enum class CameraMode { FirstPerson, ThirdPerson, Free };

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();

    void InitSkybox();
    void InitCamara();
    void InitMiniMap();
    void CameraTransform(float dt, std::vector<DSM::BlockId> containBlock);
    void ImGuiOperations(float dt);
    void PlaceDestroyBlocks();
    void DrawScene(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, const D3D11_VIEWPORT& viewport);
    void DayAndNightChange(float dt);
    void EnemyManagement();


private:
    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;		            			    // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;							    // 天空盒特效管理
    PostProcessEffect m_PostProcessEffect;						// 后处理特效管理

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度缓冲区
    std::unique_ptr<Texture2D> m_pMinimapTexture;               // 小地图纹理
    std::unique_ptr<Texture2D> m_pLitTexture;                   // 中间场景缓冲区

    std::shared_ptr<FirstPersonCamera> m_pFCamera;			    // 第一人称摄像机
    std::shared_ptr<ThirdPersonCamera> m_pTCamera;              // 第三人称摄像机
    DSM::FreeCameraController m_FCameraControl;                 // 摄像机控制器 
    DSM::FirstPersonCameraController m_FPCameraControl;         // 第一人称控制器
    DSM::ThirdCameraController m_TCameraControl;                // 第三人称控制器
    CameraMode m_CameraMode = CameraMode::Free;                 // 摄像机模式

    ComPtr<ID3D11RasterizerState> m_pRState;	                // 光栅化状态

    GameObject m_Skybox;                                        // 天空盒
    float m_SkyColor = 1.0f;                                    // 天空的颜色，模拟时间天空昼夜
    float m_SkySign = 1.0f;                                     // 天空颜色增减标志
    DirectionalLight m_DirLight[4];                             // 方向光
    float m_DiffuseSign = -1.0f;                                // 天空颜色增减标志
    float m_Diffuse = 0.5f;                                     // 方向光漫反射的系数

    std::vector<DSM::Chunk> m_Chunk;                            // 区块
    int m_Radius = 3;                                           // 区块半径
    DSM::CherryTree m_CherryTree;                               // 树

    DSM::Player m_Player;                                       // 玩家
    std::vector<DSM::Enemy> m_Enemy;                                         // 敌人
    bool m_EnemyTrack = false;                                  // 敌人追踪
    
    bool m_IsNight = false;                                     // 是否为黑夜
    bool m_FogEnabled = false;                                  // 雾效
    float m_FogStart = 5.0f;                                    // 雾效起点    
    float m_FogRange = 100.0f;                                  // 雾效范围

    bool m_FadeUsed = true;                                     // 淡入淡出
    bool m_PrintScreenStarted = false;                          // 截取当前帧
    float m_FadeCount = 0.0f;                                   // 淡出系数
    float m_FadeSign = 1.0f;                                    

    std::vector<DSM::Block> m_Dirt;                              // 泥土
    size_t m_SoilNum = 64;





};



#endif