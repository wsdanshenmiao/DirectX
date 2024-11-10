#ifndef __UPLOADBUFFER__H__
#define __UPLOADBUFFER__H__

#include "D3DUtil.h"

namespace DSM {

	template<typename T>
	class UploadBuffer
	{
	public:
		template <class P>
		using ComPtr = Microsoft::WRL::ComPtr<P>;
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer);
		UploadBuffer(const UploadBuffer& other) = delete;
		UploadBuffer(UploadBuffer&& other) = default;
		UploadBuffer& operator=(const UploadBuffer& other) = delete;
		UploadBuffer& operator=(UploadBuffer&& other) = default;
		~UploadBuffer();

		ID3D12Resource* GetResource();
		void CopyData(int elementIndex, const T& data);

	private:
		ComPtr<ID3D12Resource> m_UploadBuffer;
		BYTE* m_MappedData = nullptr;
		UINT m_ElementByteSize = 0;
		bool m_IsConstantBuffer = false;
	};


	template<typename T>
	inline UploadBuffer<T>::UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
		:m_IsConstantBuffer(isConstantBuffer) {
		m_ElementByteSize = m_IsConstantBuffer ? D3DUtil::CalcConstantBufferByteSize(sizeof(T)) : sizeof(T);
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

		ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
		// 只要还需修改资源就无需取消映射，在被GPU使用期间不可进行写操作
	}

	template<typename T>
	inline UploadBuffer<T>::~UploadBuffer()
	{
		if (m_UploadBuffer != nullptr) {
			m_UploadBuffer->Unmap(0, nullptr);
		}
		m_MappedData = nullptr;
	}

	template<typename T>
	inline ID3D12Resource* UploadBuffer<T>::GetResource()
	{
		return m_UploadBuffer.Get();
	}

	template<typename T>
	inline void UploadBuffer<T>::CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_MappedData[m_ElementByteSize * elementIndex], &data, sizeof(T));
	}



}

#endif // !__UPLOADBUFFER__H__
