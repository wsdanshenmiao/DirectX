#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <WinMin.h>
#include <d3dApp.h>
#include <EffectHelper.h>
#include <sstream>

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
    bool InitEffect();
    void UpdataTorus();

private:
    ComPtr<ID3D11Buffer> m_pTorusBuffer;
    float m_QuadEdgeTess[4] = { 30.0f, 30.0f, 30.0f, 30.0f };				// 四边形各边的细分因子
    float m_QuadInsideTess[2] = { 30.0f, 30.0f };							// 四边形内部的细分因子
    float m_Normal[3] = { 3,4,5 };
    float m_Center[3] = { 0,0,0 };

    float m_Phi = 3.14159f / 4, m_Theta = 0.0f, m_Radius = 1.2f;		// 第三人称视角摄像机参数

    ComPtr<ID3D11InputLayout> m_pInputLayout;							// 顶点输入布局
    ComPtr<ID3D11RasterizerState> m_pRSWireFrame;						// 线框光栅化

    std::unique_ptr<EffectHelper> m_pEffectHelper;						// 特效助理
};


#endif