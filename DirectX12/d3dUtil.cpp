
#include "d3dUtil.h"
#include <comdef.h>
#include <fstream>

using Microsoft::WRL::ComPtr;

DxException::DxException(HRESULT hr, const std::wstring& kwstrFunctionName, const std::wstring& kwstrFilename, int iLineNumber) : m_hrErrorCode(hr), m_wstrFunctionName(kwstrFunctionName), m_wstrFilename(kwstrFilename), m_iLineNumber(iLineNumber)
{
};

bool d3dUtil::IsKeyDown(int iVirtualKeyCode)
{
    return (GetAsyncKeyState(iVirtualKeyCode) & 0x8000) != 0;
};

ComPtr<ID3DBlob> d3dUtil::LoadBinary(const std::wstring& kwstrFilename)
{
    std::ifstream fin(kwstrFilename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    ComPtr<ID3DBlob> pBlob;
    ThrowIfFailed(D3DCreateBlob(size, pBlob.GetAddressOf()));

    fin.read((char*)pBlob->GetBufferPointer(), size);
    fin.close();

    return pBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pGraphicsCommandList, const void* kInitData, UINT64 ui64ByteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& pUploadBuffer)
{
    ComPtr<ID3D12Resource> pDefaultBuffer;

    // Create the actual default buffer resource.
    ThrowIfFailed(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ui64ByteSize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(pDefaultBuffer.GetAddressOf())));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ui64ByteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pUploadBuffer.GetAddressOf())));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = kInitData;
    subResourceData.RowPitch = ui64ByteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
	pGraphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDefaultBuffer.Get(),  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(pGraphicsCommandList, pDefaultBuffer.Get(), pUploadBuffer.Get(), 0, 0, 1, &subResourceData);
	pGraphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return pDefaultBuffer;
}

ComPtr<ID3DBlob> d3dUtil::CompileShader(const std::wstring& kwstrFilename, const D3D_SHADER_MACRO* kDefines, const std::string& kstrEntrypoint, const std::string& kstrTarget)
{
	UINT uiCompileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	uiCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> pByteCode = nullptr;
	ComPtr<ID3DBlob> pErrors;
	hr = D3DCompileFromFile(kwstrFilename.c_str(), kDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		kstrEntrypoint.c_str(), kstrTarget.c_str(), uiCompileFlags, 0, &pByteCode, &pErrors);

	if (pErrors != nullptr) 
		{ OutputDebugStringA((char*)pErrors->GetBufferPointer()); }

	ThrowIfFailed(hr);

	return pByteCode;
}

std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(m_hrErrorCode);
    std::wstring msg = err.ErrorMessage();

    return m_wstrFunctionName + L" failed in " + m_wstrFilename + L"; line " + std::to_wstring(m_iLineNumber) + L"; error: " + msg;
}


