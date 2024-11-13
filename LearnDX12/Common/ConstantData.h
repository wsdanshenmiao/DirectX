#pragma once
#ifndef __CONSTANTDATA__H__
#define __CONSTANTDATA__H__

#include "D3DUtil.h"
#include "MathHelper.h"

namespace DSM {
	struct ConstantData {};

	struct ObjectConstants : public ConstantData
	{
		DirectX::XMFLOAT4X4 World = MathHelper::Identity();
		DirectX::XMFLOAT4X4 WorldInvTranspos = MathHelper::Identity();
	};

	struct PassConstants : public ConstantData
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity();
		DirectX::XMFLOAT4X4 InvView = MathHelper::Identity();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity();
		DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity();
		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};
}

#endif // !__CONSTANTDATA__H__
