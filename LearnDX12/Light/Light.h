#pragma once
#ifndef __LIGHT__H__
#define __LIGHT__H__

#include "../Common/D3D12.h"
#include "../Common/FrameResource.h"
#include "../Common/ConstantData.h"
#include "Object.h"


namespace DSM {
	class Light : public D3D12App
	{
	public:
		Light(HINSTANCE hAppInst, const std::wstring& mainWndCaption, int clientWidth = 512, int clientHeight = 512);

		bool OnInit() override;

	protected:
		// 通过 D3D12App 继承
		void OnUpdate(const CpuTimer& timer) override;
		void OnRender(const CpuTimer& timer) override;

		void OnRenderScene();
		void WaitForGPU();
		bool ImportModel();

		bool InitResource();
		void CreateShaderBlob();
		void CreateFrameResource();
		void CreateCBV();
		void CreateRootSignature();
		void CreatePSO();


	private:
		inline static constexpr UINT FrameCount = 3;

	private:
		ComPtr<ID3D12DescriptorHeap> m_PassCbv = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_ObjCbv = nullptr;
		ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
		std::map<std::string, ComPtr< ID3D12PipelineState>> m_PSOs;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_VSByteCodes;
		std::unordered_map<std::string, ComPtr<ID3D10Blob>> m_PSByteCodes;

		std::array<std::unique_ptr<FrameResource>, FrameCount> m_FrameResources;
		FrameResource* m_CurrFrameResource;
		std::map<std::string, Object> m_Objects;

		UINT m_RenderObjCount = 0;		// 绘制物体个数
		UINT m_CurrFRIndex = 0;
	};

}

#endif // !__LIGHT__H__
