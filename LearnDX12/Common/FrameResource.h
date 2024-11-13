#pragma once
#ifndef __FRAMERESOURCE__H__
#define __FRAMERESOURCE__H__

#include "D3DUtil.h"
#include "ConstantData.h"
#include "UploadBuffer.h"

namespace DSM {

	struct FrameResource
	{
		template <class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		using PConstantBuffer = std::unique_ptr<UploadBuffer<ConstantData>>;

		FrameResource(ID3D12Device* device);
		FrameResource(const FrameResource& other) = delete;
		FrameResource& operator=(const FrameResource& other) = delete;
		~FrameResource() = default;

		void AddConstantBuffer(
			ID3D12Device* device,
			UINT byteSize,
			UINT elementSize,
			const std::string& bufferName);

		ComPtr<ID3D12CommandAllocator> m_CmdListAlloc;						// 每个帧资源都有一个命令列表分配器
		std::unordered_map<std::string, PConstantBuffer> m_ConstantBuffers;
		UINT64 m_Fence = 0;
	};
}


#endif // !__FRAMERESOURCE__H__
