#include "FrameResource.h"

DSM::FrameResource::FrameResource(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CmdListAlloc.GetAddressOf())));
}

void DSM::FrameResource::AddConstantBuffer(
	ID3D12Device* device,
	UINT byteSize,
	UINT elementSize,
	const std::string& bufferName)
{
	auto pCB = std::make_unique<UploadBuffer<ConstantData>>(device, byteSize, elementSize, true);
	m_ConstantBuffers.emplace(bufferName, std::move(pCB));
}
