#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include "Waves.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>

struct FragmentData
{
    uint32_t color;
    float depth;
};

struct FLStaticNode
{
    FragmentData data;
    uint32_t next;
};

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

private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    std::mt19937 m_RandEngine;									// 随机数生成器
    std::uniform_int_distribution<UINT> m_RowRange;				// 行索引范围
    std::uniform_int_distribution<UINT> m_ColRange;				// 列索引范围
    std::uniform_real_distribution<float> m_MagnitudeRange;		// 振幅范围

    BasicEffect m_BasicEffect;									// 对象渲染特效管理
    PostProcessEffect m_PostProcessEffect;                      // 后处理特效管理

    GameObject m_Land;											// 地面对象
    GameObject m_RedBox;										// 红色箱子
    GameObject m_YellowBox;										// 黄色箱子
    GpuWaves m_GpuWaves;                                        // GPU水波
    
    std::unique_ptr<StructuredBuffer<FLStaticNode>> m_pFLStaticNodeBuffer;    // 静态链表缓冲区
    std::unique_ptr<ByteAddressBuffer> m_pStartOffsetBuffer;    // 起始偏移缓冲区

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度纹理
    std::unique_ptr<Texture2D> m_pLitTexture;                   // 场景渲染缓冲区(不透明+透明)
    std::unique_ptr<Texture2D> m_pTempTexture;                  // 临时缓冲区

    float m_BaseTime = 0.0f;								    // 控制水波生成的基准时间

    bool m_EnabledFog = true;									// 开启雾效
    bool m_EnabledOIT = true;									// 开启OIT
    int m_BlurMode = 1;										    // 0-Sobel, 1-Blur

    float m_BlurSigma = 2.5f;									// 模糊用的sigma值
    int m_BlurRadius = 5;										// 模糊用的半径
    int m_BlurTimes = 1;										// 模糊次数

    std::shared_ptr<Camera> m_pCamera;						    // 摄像机
};


#endif