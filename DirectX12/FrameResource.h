#ifndef FRAMERESOURCE_H_INCLUDED
#define FRAMERESOURCE_H_INCLUDED
#pragma once

#include "D3DUtility.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 m_worldMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_texureTransformMatrix = MathHelper::Identity4x4();
}; // ObjectConstants

struct PassConstants
{
	// Matrices
	DirectX::XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseViewMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseProjectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_viewProjectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseViewProjectionMatrix = MathHelper::Identity4x4();
	
	// XMFloats
	DirectX::XMFLOAT4 m_ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_eyePositionW = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT2 m_renderTargetSizeMatrix = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 m_inverseRenderTargetSizeMatrix = { 0.0f, 0.0f };

	// Floats
	float m_fConstantBufferPerObjectPad1 = 0.0f;
	float m_fNearZ = 0.0f;
	float m_fFarZ = 0.0f;
	float m_fTotalTime = 0.0f;
	float m_fDeltaTime = 0.0f;

	// Light array
	Light Lights[MaxLights];

}; // PassConsants

struct Vertex
{
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT4 m_color;
	DirectX::XMFLOAT3 m_normal;
}; // Vertex

struct FrameResource
{
public:

	FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount, UINT uiMaterialCount);
	FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pxCommandListAllocator;

	
	std::unique_ptr<UploadBuffer<PassConstants>> m_pxPassConstantBuffer;
	std::unique_ptr<UploadBuffer<MaterialConstants>> m_pxMaterialConstantBuffer;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_pxObjectConstantBuffer;

	UINT64 m_ui64Fence;
}; // FrameResource

#endif // FRAMERESOURCE_H_INCLUDED