#ifndef FRAMERESOURCE_H_INCLUDED
#define FRAMERESOURCE_H_INCLUDED
#pragma once

#include "D3DUtility.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 m_worlMatrix = MathHelper::Identity4x4();
}; // ObjectConstants

struct PassConstants
{
	DirectX::XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseViewMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseProjectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_viewProjectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_inverseViewProjectionMatrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 m_eyePositionW = {0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT2 m_renderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 m_inverseRenderTargetSize = { 0.0f, 0.0f };

	float m_fConstantPassObjectPad1 = 0.0f;
	float m_fNearZ = 0.0f;
	float m_fFarZ = 0.0f;
	float m_fTotalTime = 0.0f;
	float m_fDeltaTime = 0.0f;

}; // PassConsants

struct Vertex
{
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT4 m_color;
}; // Vertex

struct FrameResource
{
	public:

		FrameResource(ID3D12Device* pDevice, UINT uiPassCount, UINT uiObjectCount);
		FrameResource(const FrameResource& kFrameResource) = delete;
		FrameResource& operator=(const FrameResource& kFrameResource) = delete;
		~FrameResource();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pxCommandListAllocator;
		std::unique_ptr<UploadBuffer<PassConstants>> m_pxPassConstantBuffer;
		std::unique_ptr<UploadBuffer<ObjectConstants>> m_pxObjectConstantBuffer;

		UINT64 m_ui64Fence;
}; // FrameResource

#endif // FRAMERESOURCE_H_INCLUDED