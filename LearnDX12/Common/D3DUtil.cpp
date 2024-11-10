#include "D3DUtil.h"

using Microsoft::WRL::ComPtr;

namespace DSM {

	// 用于创建默认缓冲区 D3D12_HEAP_TYPE_DEFAULT
	ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& upLoadBuffer)
	{
		ComPtr<ID3D12Resource> defaultBuffer;

		// 创建默认缓冲区资源
		auto heapPropertiesDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto resourceState = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapPropertiesDefault,	// 资源堆属性
			D3D12_HEAP_FLAG_NONE,
			&resourceState,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		));

		// 创建一个中介的上传堆来将Cpu内存复制到默认缓冲区
		auto heapPropertiesUpLoad = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapPropertiesUpLoad,
			D3D12_HEAP_FLAG_NONE,
			&resourceState,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(upLoadBuffer.GetAddressOf())));

		// 描述要复制到默认缓冲区的资源
		D3D12_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pData = initData;	// 指向包含子资源数据的内存块的指针
		subResourceData.RowPitch = byteSize;// 子资源数据的行间距
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default bufferresource.
		// At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediateupload heap.
		// Then, using ID3D12CommandList::CopyBufferRegion or CopyTextureRegion,
		// the intermediate upload heap data will be copiedto mBuffer.
		auto commonToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &commonToCopyDest);
		UpdateSubresources<1>(
			cmdList,
			defaultBuffer.Get(),	// 目标资源
			upLoadBuffer.Get(),		// 中介缓冲区
			0, 0, 1,
			&subResourceData);		// 源缓冲区
		auto copyDestToCommon = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &copyDestToCommon);

		// UploadBuffer不能立刻销毁，因为提交的命令列表可能尚未执行
		return defaultBuffer;
	}

	UINT D3DUtil::CalcConstantBufferByteSize(UINT byteSize) noexcept
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}

	ComPtr<ID3DBlob> D3DUtil::CompileShader(
		const WCHAR* fileName,
		const D3D_SHADER_MACRO* defines,
		const std::string& enteryPoint,
		const std::string& target,
		const WCHAR* outputFileName)
	{
		ComPtr<ID3DBlob> byteCode = nullptr;
		ComPtr<ID3DBlob> errors = nullptr;
		UINT compilFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG) 
		compilFlags |= D3DCOMPILE_DEBUG;
		compilFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		auto hr = D3DCompileFromFile(
			fileName,
			defines,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			enteryPoint.c_str(),
			target.c_str(),
			compilFlags,
			0,
			byteCode.GetAddressOf(),
			errors.GetAddressOf());

		if (errors != nullptr) {
			OutputDebugStringA(static_cast<char*>(errors->GetBufferPointer()));
		}
		ThrowIfFailed(hr);

		if (outputFileName != nullptr) {
			ThrowIfFailed(D3DWriteBlobToFile(byteCode.Get(), outputFileName, TRUE));
		}

		return byteCode;
	}

	DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
		ErrorCode(hr),
		FunctionName(functionName),
		Filename(filename),
		LineNumber(lineNumber) {}


	std::wstring DxException::ToString()const
	{
		// Get the string description of the error code.
		_com_error err(ErrorCode);
		std::wstring msg = err.ErrorMessage();

		return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
	}


}
