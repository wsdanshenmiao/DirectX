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
#include "Block.h"
#include "BlockModel.h"
#include "Chunk.h"
#include "Player.h"

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

private:
    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;		            			    // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;							    // 天空盒特效管理

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度缓冲区

    std::shared_ptr<FirstPersonCamera> m_pCamera;			    // 摄像机
    FirstPersonCameraController m_CameraController;             // 摄像机控制器 
    CameraMode m_CameraMode;                                    // 摄像机模式

    ComPtr<ID3D11RasterizerState> m_pRState;	                // 光栅化状态

    GameObject m_Skybox;                                        // 天空盒
    std::vector<Chunk> m_Chunk;                                 // 区块
    Player m_Player;                                            // 玩家
    
    std::vector<Block> m_Dirt;                                  // 泥土

};



#endif