#ifndef UPLOADBUFER_H_INCLUDED
#define UPLOADBUFER_H_INCLUDED
#pragma once

#include "D3DUtility.h"

template<typename T>
class UploadBuffer
{
	public:
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : m_bIsConstantBuffer(isConstantBuffer)
		{
			m_uiElementByteSize = sizeof(T);
	
			// Constant buffer elements need to be multiples of 256 bytes.
			// This is because the hardware can only view constant data 
			// at m*256 byte offsets and of n*256 byte lengths. 
			// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
			// UINT64 OffsetInBytes; // multiple of 256
			// UINT   SizeInBytes;   // multiple of 256
			// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
			if (isConstantBuffer)
				m_uiElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));
	
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_uiElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_pxUploadBuffer)));
	
			ThrowIfFailed(m_pxUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
	
			// We do not need to unmap until we are done with the resource.  However, we must not write to
			// the resource while it is in use by the GPU (so we must use synchronization techniques).
		}
	
		UploadBuffer(const UploadBuffer& rhs) = delete;
		UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
		~UploadBuffer()
		{
			if (m_pxUploadBuffer != nullptr)
				m_pxUploadBuffer->Unmap(0, nullptr);
	
			m_mappedData = nullptr;
		}
	
		ID3D12Resource* Resource() const
		{
			return m_pxUploadBuffer.Get();
		}
	
		void CopyData(int elementIndex, const T& data)
		{
			memcpy(&m_mappedData[elementIndex * m_uiElementByteSize], &data, sizeof(T));
		};
	
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pxUploadBuffer;
		BYTE* m_mappedData = nullptr;
	
		UINT m_uiElementByteSize = 0;
		bool m_bIsConstantBuffer = false;
};

#endif // UPLOADBUFER_H_INCLUDED