#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount, UINT uiMaterialCount, UINT uiWaveVertexCount) : m_pxPassConstantBuffer(nullptr), m_pxMaterialConstantBuffer(nullptr), m_pxObjectConstantBuffer(nullptr), m_pxWavesVertexBuffer(nullptr)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pxCommandListAllocator.GetAddressOf())));

	m_pxPassConstantBuffer = std::make_unique<UploadBuffer<PassConstants>>(pDevice, uiPassCount, true);
	m_pxMaterialConstantBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(pDevice, uiMaterialCount, true);
	m_pxObjectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, uiObjectCount, true);

	m_pxWavesVertexBuffer = std::make_unique<UploadBuffer<Vertex>>(pDevice, uiWaveVertexCount, false);
}

FrameResource::FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount, UINT uiMaterialCount) : m_pxPassConstantBuffer(nullptr), m_pxMaterialConstantBuffer(nullptr), m_pxObjectConstantBuffer(nullptr)
{
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pxCommandListAllocator.GetAddressOf())));

  //  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    m_pxPassConstantBuffer = std::make_unique<UploadBuffer<PassConstants>>(pDevice, uiPassCount, true);
    m_pxMaterialConstantBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(pDevice, uiMaterialCount, true);
    m_pxObjectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, uiObjectCount, true);
}

FrameResource::~FrameResource()
{

}