#ifndef FRAMERESOURCE_H_INCLUDED
#define FRAMERESOURCE_H_INCLUDED
#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 m_worldMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_textureTransformMatrix = MathHelper::Identity4x4();
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 m_inverseViewMatrix = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 m_projection = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseProjectionMatrix = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 m_viewProjectionMatrix = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 m_inverseViewProjectionMatrix = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 m_eyePosition = { 0.0f, 0.0f, 0.0f };
    float m_fConstantBufferPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 m_renderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 m_inverseRenderTargetSize = { 0.0f, 0.0f };
    float m_fNearZ = 0.0f;
    float m_fFarZ = 0.0f;
    float m_fTotalTime = 0.0f;
    float m_fDeltaTime = 0.0f;

    DirectX::XMFLOAT4 m_ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light m_lights[MaxLights];
};

struct Vertex
{
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_normal;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
	public:

		FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount, UINT uiMaterialCount);
		FrameResource(const FrameResource& kFrameResource) = delete;
		FrameResource& operator=(const FrameResource& kFrameResource) = delete;
		~FrameResource();

		// We cannot reset the allocator until the GPU is done processing the commands.
		// So each frame needs their own allocator.
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pxCommandListAllocator;

		// We cannot update a cbuffer until the GPU is done processing the commands
		// that reference it.  So each frame needs their own cbuffers.
		// std::unique_ptr<UploadBuffer<FrameConstants>> FrameCB = nullptr;
		std::unique_ptr<UploadBuffer<PassConstants>> m_pxPassConstantBuffer = nullptr;
		std::unique_ptr<UploadBuffer<MaterialConstants>> m_pxMaterialConstantBuffer = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> m_pxObjectConstantBuffer = nullptr;

		// Fence value to mark commands up to this fence point.  This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64 m_ui64Fence = 0;
};

#endif // FRAMERESOURCE_H_INCLUDED