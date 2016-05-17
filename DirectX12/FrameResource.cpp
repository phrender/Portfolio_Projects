#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount) : m_pxPassConstantBuffer(nullptr), m_pxObjectConstantBuffer(nullptr), m_pxCommandListAllocator(nullptr), m_ui64Fence(0)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pxCommandListAllocator.GetAddressOf())));

	m_pxPassConstantBuffer = std::make_unique<UploadBuffer<PassConstants>>(pDevice, uiPassCount, true);
	m_pxObjectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, uiObjectCount, true);
};

FrameResource::~FrameResource()
{
};
