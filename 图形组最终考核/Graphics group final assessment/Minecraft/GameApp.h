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
#include "Block.h"
#include "BlockModel.h"
#include "Chunk.h"
#include "Player.h"
#include "MyCameraControl.h"

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
    void CameraTransform(float dt);

private:
    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;		            			    // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;							    // 天空盒特效管理

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度缓冲区

    std::shared_ptr<FirstPersonCamera> m_pFCamera;			    // 第一人称摄像机
    std::shared_ptr<ThirdPersonCamera> m_pTCamera;              // 第三人称摄像机
    DSM::FreeCameraController m_FCameraControl;                 // 摄像机控制器 
    DSM::FirstPersonCameraController m_FPCameraControl;
    DSM::ThirdCameraController m_TCameraControl;
    CameraMode m_CameraMode = CameraMode::Free;                                    // 摄像机模式

    ComPtr<ID3D11RasterizerState> m_pRState;	                // 光栅化状态

    GameObject m_Skybox;                                        // 天空盒
    std::vector<DSM::Chunk> m_Chunk;                                 // 区块
    DSM::Player m_Player;                                            // 玩家
    
    bool m_IsNight = false;
    bool m_FogEnabled = false;                                  //  雾效
    float m_FogStart = 5.0f;
    float m_FogRange = 75.0f;                                    // 雾效范围

    bool m_FadeUsed = true;                                     // 淡入淡出
    bool m_PrintScreenStarted = false;                          // 截取当前帧
    float m_FadeCount = 0.0f;                                   
    float m_FadeSign = 1.0f;

    std::vector<DSM::Block> m_Dirt;                                  // 泥土

};



#endif