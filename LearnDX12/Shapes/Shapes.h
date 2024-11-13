#pragma once
#ifndef __SHAPES__H__
#define __SHAPES__H__

#include "../Common/D3D12.h"
#include "../Common/Mesh.h"
#include "../Common/FrameResource.h"

namespace DSM {

	class Shapes : public D3D12App
	{
	public:
		Shapes(HINSTANCE hAppInst, const std::wstring& mainWndCaption, int clientWidth = 512, int clientHeight = 512);
		Shapes(const Shapes& other) = delete;
		Shapes& operator=(const Shapes& other) = delete;
		virtual ~Shapes() = default;

		bool OnInit();

	private:
		void OnResize();
		void OnUpdate(const CpuTimer& timer);
		void OnRender(const CpuTimer& timer);
		void OnRenderScene(const CpuTimer& timer);

		bool InitResource();
		bool CreateBox();
		bool CreateRootSignature();
		bool CreatePSO();
		void WaitForGpu();
		void UpdateFrameResource(const CpuTimer& timer);

	private:
		inline static constexpr UINT FrameCount = 3;

	private:
		ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;			// 根签名
		ComPtr<ID3D12PipelineState> m_PSO = nullptr;
		ComPtr<ID3DBlob> m_VSByteCode = nullptr;
		ComPtr<ID3DBlob> m_PSByteCode = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_CbvHeap = nullptr;

		std::unique_ptr<Geometry::MeshData> m_Box;
		std::array<std::unique_ptr<FrameResource>, FrameCount> m_FrameResources;	// 帧资源循环数组
		FrameResource* m_CurrFrameResource = nullptr;
		UINT m_CurrFRIndex = 0;
	};

}

#endif // !__SHAPES__H__
