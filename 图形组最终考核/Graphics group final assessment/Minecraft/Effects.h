//***************************************************************************************
// Effects.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 简易特效管理框架
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include <LightHelper.h>
#include <RenderStates.h>

#include <GameObject.h>

#include <Buffer.h>
#include <IEffect.h>
#include <Material.h>
#include <MeshData.h>
#include <LightHelper.h>


class BasicEffect : public IEffect, public IEffectTransform,
    public IEffectMaterial, public IEffectMeshData
{
public:
    struct InstancedData
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4X4 worldInvTranspose;
    };

public:
    BasicEffect();
    virtual ~BasicEffect() override;

    BasicEffect(BasicEffect&& moveFrom) noexcept;
    BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

    // 获取单例
    static BasicEffect& Get();

    // 初始化所需资源
    bool InitAll(ID3D11Device* device);

    //
    // IEffectTransform
    //

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;


    //
    // IEffectMaterial
    //

    void SetMaterial(const Material& material) override;

    //
    // IEffectMeshData
    //

    MeshDataInput GetInputData(const MeshData& meshData) override;

    //
    // BasicEffect
    //

    // 默认状态来绘制
    void SetRenderDefault();

    // 公告板绘制
    void SetRenderBillboard(ID3D11DeviceContext* deviceContext, bool enableAlphaToCoverage);

    // 绘制实例
    void DrawInstanced(ID3D11DeviceContext* deviceContext, Buffer& instancedBuffer, const GameObject& object, uint32_t numObjects);

    // 各种类型灯光允许的最大数目
    static const int maxLights = 5;

    void SetDirLight(uint32_t pos, const DirectionalLight& dirLight);
    void SetPointLight(uint32_t pos, const PointLight& pointLight);
    void SetSpotLight(uint32_t pos, const SpotLight& spotLight);

    void SetEyePos(const DirectX::XMFLOAT3& eyePos);

    void SetFogState(bool isOn);
    void SetFogStart(float fogStart);
    void SetFogColor(const DirectX::XMFLOAT4& fogColor);
    void SetFogRange(float fogRange);

    // 应用常量缓冲区和纹理资源的变更
    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

class PostProcessEffect
{
public:
    PostProcessEffect();
    ~PostProcessEffect();

    PostProcessEffect(PostProcessEffect&& moveFrom) noexcept;
    PostProcessEffect& operator=(PostProcessEffect&& moveFrom) noexcept;

    // 获取单例
    static PostProcessEffect& Get();

    bool InitAll(ID3D11Device* device);

    // 渐变特效
    void RenderScreenFade(
        ID3D11DeviceContext* deviceContext,
        ID3D11ShaderResourceView* input,
        ID3D11RenderTargetView* output,
        const D3D11_VIEWPORT& vp,
        float fadeAmount);


    // 小地图

    void SetVisibleRange(float range);
    void SetEyePos(const DirectX::XMFLOAT3& eyePos);
    void SetMinimapRect(const DirectX::XMFLOAT4& rect); // (Left, Front, Right, Back)
    void RenderMinimap(
        ID3D11DeviceContext* deviceContext,
        ID3D11ShaderResourceView* input,
        ID3D11RenderTargetView* output,
        const D3D11_VIEWPORT& vp
    );

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};


class SkyboxEffect : public IEffect, public IEffectTransform,
    public IEffectMaterial, public IEffectMeshData
{
public:
    SkyboxEffect();
    virtual ~SkyboxEffect() override;

    SkyboxEffect(SkyboxEffect&& moveFrom) noexcept;
    SkyboxEffect& operator=(SkyboxEffect&& moveFrom) noexcept;

    // 获取单例
    static SkyboxEffect& Get();

    // 初始化所需资源
    bool InitAll(ID3D11Device* device);

    //
    // IEffectTransform
    //

    // 无用
    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;

    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

    //
    // IEffectMaterial
    //

    void SetMaterial(const Material& material) override;

    void SetSkyCount(ID3D11DeviceContext* deviceContext, float skyCount);

    //
    // IEffectMeshData
    //

    MeshDataInput GetInputData(const MeshData& meshData) override;

    //
    // SkyboxEffect
    //

    void SetRenderDefault();

    // 应用常量缓冲区和纹理资源的变更
    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};




#endif
