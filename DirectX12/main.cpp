#include "D3DApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int g_kiNumFrameResources = 3;

struct RenderItem
{

	RenderItem() = default;

	XMFLOAT4X4 m_worldMatrix = MathHelper::Identity4x4();
	XMFLOAT4X4 m_textureTransformMatrix = MathHelper::Identity4x4();

	int m_iNumFramesDirty = g_kiNumFrameResources;

	Material* m_pxMaterial = nullptr;
	MeshGeometry* m_pxGeometry = nullptr;
	
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_uiObjectConstantBufferIndex = -1;
	UINT m_uiIndexCount = 0;
	UINT m_uiStartIndexLocation = 0;
	
	int m_iBaseVertexLocation = 0;

}; // RenderItem

class InitDirect3DApp : public D3DApp
{
	public:

		InitDirect3DApp(HINSTANCE hInstance);
		InitDirect3DApp(const InitDirect3DApp& kInitDirect3DApp) = delete;
		~InitDirect3DApp();

		InitDirect3DApp& operator=(const InitDirect3DApp& kInitDirect3DApp) = delete;

		virtual bool Initialize() override;

	private:

		virtual void OnResize()override;
		virtual void Update(const GameTimer& gt) override;
		virtual void Draw(const GameTimer& gt) override;

		virtual void OnMouseDown(WPARAM btnState, int iPositionX, int iPositionY) override;
		virtual void OnMouseUp(WPARAM btnState, int iPositionX, int iPositionY) override;
		virtual void OnMouseMove(WPARAM btnState, int iPositionX, int iPositionY) override;

		void OnKeyboardInput(const GameTimer& kGameTimer);
		void UpdateCamera(const GameTimer& kGameTimer);
		void AnimateMaterials(const GameTimer& kGameTimer);
		void UpdateObjectConstantBuffers(const GameTimer& kGameTimer);
		void UpdateMaterialConstantBuffers(const GameTimer& kGameTimer);
		void UpdateMainPassCB(const GameTimer& kGameTimer);

		void BuildDescriptorHeaps();
		void BuildConstantBufferViews();
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildShapeGeometry();
		void BuildSkullGeometry();
		void BuildPipelineStateObjects();
		void BuildFrameResources();
		void BuildMaterials();
		void BuildRenderItems();
		void DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems);
		
	private:

		FrameResource* m_pxCurrentFrameResource = nullptr;
		
		ComPtr<ID3D12RootSignature> m_pxRootSignature = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_pxConstantBufferViewHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_pxShaderResourceViewDescriptorHeap = nullptr;
		ComPtr<ID3D12PipelineState> m_pxOpaquePipelineStateObject = nullptr;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_material;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_texture;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_pipelineStateObjects;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
		std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
		std::vector<RenderItem*> m_opaqueRenderItems;
		std::vector<std::unique_ptr<FrameResource>> m_frameResources;

		PassConstants m_mainPassConstantBuffer;

		int m_iCurrentFrameResourceIndex = 0;

		UINT m_uiConstantBufferViewShaderResourceViewDescSize = 0;
		UINT m_uiPassConstantBufferViewOffset = 0;

		bool m_bIsWireframe = false;

		XMFLOAT3 m_eyePosition = {0.0f, 0.0f, 0.0f};
		XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
		XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();

		float m_fTheta = 1.5f * XM_PI;
		float m_fPhi = 0.2f * XM_PI;
		float m_fRadius = 15.0f;

		POINT m_lastMousePos;
}; // InitDirect3DApp

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
};

InitDirect3DApp::~InitDirect3DApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	};
};

bool InitDirect3DApp::Initialize()
{
	if (!D3DApp::Initialize())
		{ return false; };

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	m_uiConstantBufferViewShaderResourceViewDescSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildSkullGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	//BuildDescriptorHeaps();
	//BuildConstantBufferViews();
	BuildPipelineStateObjects();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	return true;
};

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
};

void InitDirect3DApp::Update(const GameTimer& kGameTimer)
{
	OnKeyboardInput(kGameTimer);
	UpdateCamera(kGameTimer);

	m_iCurrentFrameResourceIndex = (m_iCurrentFrameResourceIndex + 1) % g_kiNumFrameResources;
	m_pxCurrentFrameResource = m_frameResources[m_iCurrentFrameResourceIndex].get();

	if (m_pxCurrentFrameResource->m_ui64Fence != 0 && mFence->GetCompletedValue() < m_pxCurrentFrameResource->m_ui64Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(m_pxCurrentFrameResource->m_ui64Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimateMaterials(kGameTimer);
	UpdateObjectConstantBuffers(kGameTimer);
	UpdateMaterialConstantBuffers(kGameTimer);
	UpdateMainPassCB(kGameTimer);
};

void InitDirect3DApp::Draw(const GameTimer& kGameTimer)
{
	auto cmdListAlloc = m_pxCurrentFrameResource->m_pxCommandListAllocator;

	ThrowIfFailed(cmdListAlloc->Reset());

	/*
	if (m_bIsWireframe)
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), m_pipelineStateObjects["opaque_wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), m_pipelineStateObjects["opaque"].Get()));
	};
	*/

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_pxConstantBufferViewHeap.Get() };
	//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_pxRootSignature.Get());

	/*
	int passCbvIndex = m_uiPassConstantBufferViewOffset + m_iCurrentFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pxConstantBufferViewHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);
	*/

	auto passConstantBuffer = m_pxCurrentFrameResource->m_pxPassConstantBuffer->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passConstantBuffer->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), m_opaqueRenderItems);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	m_pxCurrentFrameResource->m_ui64Fence = ++mCurrentFence;

	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
};

void InitDirect3DApp::OnMouseDown(WPARAM btnState, int iPositionX, int iPositionY)
{
	m_lastMousePos.x = iPositionX;
	m_lastMousePos.y = iPositionY;

	SetCapture(mhMainWnd);
};

void InitDirect3DApp::OnMouseUp(WPARAM btnState, int iPositionX, int iPositionY)
{
	ReleaseCapture();
};

void InitDirect3DApp::OnMouseMove(WPARAM btnState, int iPositionX, int iPositionY)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(iPositionX - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(iPositionY - m_lastMousePos.y));

		m_fTheta += dx;
		m_fPhi += dy;

		m_fPhi = MathHelper::Clamp(m_fPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.05f*static_cast<float>(iPositionX - m_lastMousePos.x);
		float dy = 0.05f*static_cast<float>(iPositionY - m_lastMousePos.y);

		m_fRadius += dx - dy;

		m_fRadius = MathHelper::Clamp(m_fRadius, 5.0f, 150.0f);
	};

	m_lastMousePos.x = iPositionX;
	m_lastMousePos.y = iPositionY;
}

void InitDirect3DApp::OnKeyboardInput(const GameTimer& kGameTimer)
{
	if (GetAsyncKeyState('1') & 0x8000)
		m_bIsWireframe = true;
	else
		m_bIsWireframe = false;
};

void InitDirect3DApp::UpdateCamera(const GameTimer& kGameTimer)
{
	m_eyePosition.x = m_fRadius*sinf(m_fPhi)*cosf(m_fTheta);
	m_eyePosition.z = m_fRadius*sinf(m_fPhi)*sinf(m_fTheta);
	m_eyePosition.y = m_fRadius*cosf(m_fPhi);

	XMVECTOR pos = XMVectorSet(m_eyePosition.x, m_eyePosition.y, m_eyePosition.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_viewMatrix, view);
};

void InitDirect3DApp::AnimateMaterials(const GameTimer& kGameTimer)
{
};

void InitDirect3DApp::UpdateObjectConstantBuffers(const GameTimer& kGameTimer)
{
	auto currObjectCB = m_pxCurrentFrameResource->m_pxObjectConstantBuffer.get();
	for (auto& e : m_allRenderItems)
	{
		if (e->m_iNumFramesDirty > 0)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&e->m_worldMatrix);
			XMMATRIX textureTransformationMatrix = XMLoadFloat4x4(&e->m_textureTransformMatrix);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.m_worldMatrix, XMMatrixTranspose(worldMatrix));

			currObjectCB->CopyData(e->m_uiObjectConstantBufferIndex, objConstants);

			e->m_iNumFramesDirty--;
		};
	};
};

void InitDirect3DApp::UpdateMaterialConstantBuffers(const GameTimer& kGameTimer)
{
	auto pCurrentMaterialConstantBuffer = m_pxCurrentFrameResource->m_pxMaterialConstantBuffer.get();

	for (auto& e : m_material)
	{
		Material* pMaterial = e.second.get();

		if (pMaterial->NumFramesDirty > 0)
		{
			XMMATRIX materialTransformMatrix = XMLoadFloat4x4(&pMaterial->MatTransform);

			MaterialConstants materialConstants;
			ZeroMemory(&materialConstants, sizeof(MaterialConstants));
			materialConstants.DiffuseAlbedo = pMaterial->DiffuseAlbedo;
			materialConstants.FresnelR0 = pMaterial->FresnelR0;
			materialConstants.Roughness = pMaterial->Roughness;

			XMStoreFloat4x4(&materialConstants.MatTransform, materialTransformMatrix);
			
			pCurrentMaterialConstantBuffer->CopyData(pMaterial->MatCBIndex, materialConstants);

			pMaterial->NumFramesDirty--;
		};
	};
};

void InitDirect3DApp::UpdateMainPassCB(const GameTimer& kGameTimer)
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);

	XMMATRIX viewProj = XMMatrixMultiply(viewMatrix, projectionMatrix);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(viewMatrix), viewMatrix);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(projectionMatrix), projectionMatrix);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_viewMatrix, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseViewMatrix, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_projectionMatrix, XMMatrixTranspose(projectionMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseProjectionMatrix, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_viewProjectionMatrix, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseViewProjectionMatrix, XMMatrixTranspose(invViewProj));

	m_mainPassConstantBuffer.m_eyePositionW = m_eyePosition;
	m_mainPassConstantBuffer.m_renderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	m_mainPassConstantBuffer.m_inverseRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	m_mainPassConstantBuffer.m_fNearZ = 1.0f;
	m_mainPassConstantBuffer.m_fFarZ = 1000.0f;
	m_mainPassConstantBuffer.m_fTotalTime = kGameTimer.TotalTime();
	m_mainPassConstantBuffer.m_fDeltaTime = kGameTimer.DeltaTime();
	m_mainPassConstantBuffer.m_ambientLight = {0.25f, 0.25f, 0.35f, 1.0f};
	m_mainPassConstantBuffer.Lights[0].Direction = {0.57735f, -0.57735f, 0.57735f};
	m_mainPassConstantBuffer.Lights[0].Strength = {0.6f, 0.6f, 0.6f};
	m_mainPassConstantBuffer.Lights[1].Direction = {0.57735f, -0.57735f, 0.57735f };
	m_mainPassConstantBuffer.Lights[1].Strength = {0.3f, 0.3f, 0.3f };
	m_mainPassConstantBuffer.Lights[2].Direction = {0.0f, -0.707f, -0.707f};
	m_mainPassConstantBuffer.Lights[2].Strength = {0.15f, 0.15f, 0.15f};

	auto currPassCB = m_pxCurrentFrameResource->m_pxPassConstantBuffer.get();
	currPassCB->CopyData(0, m_mainPassConstantBuffer);
};

void InitDirect3DApp::BuildDescriptorHeaps()
{
	UINT objCount = (UINT)m_opaqueRenderItems.size();

	UINT numDescriptors = (objCount + 1) * g_kiNumFrameResources;

	m_uiPassConstantBufferViewOffset = objCount * g_kiNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_pxConstantBufferViewHeap)));
};

void InitDirect3DApp::BuildConstantBufferViews()
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	UINT objCount = (UINT)m_opaqueRenderItems.size();

	for (int frameIndex = 0; frameIndex < g_kiNumFrameResources; ++frameIndex)
	{
		auto objectCB = m_frameResources[frameIndex]->m_pxObjectConstantBuffer->Resource();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			cbAddress += i*objCBByteSize;

			int heapIndex = frameIndex*objCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pxConstantBufferViewHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		};
	};

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	for (int frameIndex = 0; frameIndex < g_kiNumFrameResources; ++frameIndex)
	{
		auto passCB = m_frameResources[frameIndex]->m_pxPassConstantBuffer->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		int heapIndex = m_uiPassConstantBufferViewOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pxConstantBufferViewHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	};
};

void InitDirect3DApp::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	};
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_pxRootSignature.GetAddressOf())));
};

void InitDirect3DApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1", NULL, NULL
	};

	m_shaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		//{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
};

void InitDirect3DApp::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.m_vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.m_vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.m_vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.m_indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.m_indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.m_indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.m_uiIndexCount = (UINT)box.m_indices32.size();
	boxSubmesh.m_uiStartIndexLocation = boxIndexOffset;
	boxSubmesh.m_iBaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.m_uiIndexCount = (UINT)grid.m_indices32.size();
	gridSubmesh.m_uiStartIndexLocation = gridIndexOffset;
	gridSubmesh.m_iBaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.m_uiIndexCount = (UINT)sphere.m_indices32.size();
	sphereSubmesh.m_uiStartIndexLocation = sphereIndexOffset;
	sphereSubmesh.m_iBaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.m_uiIndexCount = (UINT)cylinder.m_indices32.size();
	cylinderSubmesh.m_uiStartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.m_iBaseVertexLocation = cylinderVertexOffset;

	auto totalVertexCount =
		box.m_vertices.size() +
		grid.m_vertices.size() +
		sphere.m_vertices.size() +
		cylinder.m_vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = box.m_vertices[i].m_position;
		vertices[k].m_normal = box.m_vertices[i].m_normal;
		vertices[k].m_color = XMFLOAT4(DirectX::Colors::DarkGreen);
	}

	for (size_t i = 0; i < grid.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = grid.m_vertices[i].m_position;
		vertices[k].m_normal = grid.m_vertices[i].m_normal;
		vertices[k].m_color = XMFLOAT4(DirectX::Colors::ForestGreen);
	}

	for (size_t i = 0; i < sphere.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = sphere.m_vertices[i].m_position;
		vertices[k].m_normal = sphere.m_vertices[i].m_normal;
		vertices[k].m_color = XMFLOAT4(DirectX::Colors::Crimson);
	}

	for (size_t i = 0; i < cylinder.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = cylinder.m_vertices[i].m_position;
		vertices[k].m_normal = cylinder.m_vertices[i].m_normal;
		vertices[k].m_color = XMFLOAT4(DirectX::Colors::SteelBlue);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT kuiVertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT kuiIndexBufferByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(kuiVertexBufferByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), kuiVertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(kuiIndexBufferByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), kuiIndexBufferByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), kuiVertexBufferByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), kuiIndexBufferByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = kuiVertexBufferByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = kuiIndexBufferByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	m_geometries[geo->Name] = std::move(geo);
};

void InitDirect3DApp::BuildSkullGeometry()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(NULL, L"Failed to load skull model", L"Model Load Error!", MB_OK);
		return;
	};

	UINT uiVertexCount = 0;
	UINT uiTextureCount = 0;
	std::string strIgnore;

	fin >> strIgnore >> uiVertexCount;
	fin >> strIgnore >> uiTextureCount;
	fin >> strIgnore >> strIgnore >> strIgnore >> strIgnore;

	std::vector<Vertex> vertices(uiVertexCount);
	for (UINT i = 0; i < uiVertexCount; ++i)
	{
		fin >> vertices[i].m_position.x >> vertices[i].m_position.y >> vertices[i].m_position.z;
		fin >> vertices[i].m_normal.x >> vertices[i].m_normal.y >> vertices[i].m_normal.z;
	};

	fin >> strIgnore;
	fin >> strIgnore;
	fin >> strIgnore;

	std::vector<std::int32_t> indices(3 * uiTextureCount);
	for (UINT i = 0; i < uiTextureCount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	};

	fin.close();

	const UINT kuiVertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT kuiIndexBufferByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto pGeometry = std::make_unique<MeshGeometry>();
	pGeometry->Name = "skullGeometry";

	ThrowIfFailed(D3DCreateBlob(kuiVertexBufferByteSize, &pGeometry->VertexBufferCPU));
	CopyMemory(pGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), kuiVertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(kuiIndexBufferByteSize, &pGeometry->IndexBufferCPU));
	CopyMemory(pGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), kuiIndexBufferByteSize);

	pGeometry->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), kuiVertexBufferByteSize, pGeometry->VertexBufferUploader);
	pGeometry->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), kuiIndexBufferByteSize, pGeometry->IndexBufferUploader);

	pGeometry->VertexByteStride = sizeof(Vertex);
	pGeometry->VertexBufferByteSize = kuiVertexBufferByteSize;

	pGeometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	pGeometry->IndexBufferByteSize = kuiIndexBufferByteSize;

	SubmeshGeometry submeshGeometry;
	ZeroMemory(&submeshGeometry, sizeof(SubmeshGeometry));
	submeshGeometry.m_uiIndexCount = (UINT)indices.size();
	submeshGeometry.m_uiStartIndexLocation = 0;
	submeshGeometry.m_iBaseVertexLocation = 0;
	pGeometry->DrawArgs["skull"] = submeshGeometry;

	m_geometries[pGeometry->Name] = std::move(pGeometry);
}

void InitDirect3DApp::BuildPipelineStateObjects()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	opaquePsoDesc.pRootSignature = m_pxRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()), 
		m_shaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()), 
		m_shaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_pxOpaquePipelineStateObject)));
};

void InitDirect3DApp::BuildFrameResources()
{
	for (int i = 0; i < g_kiNumFrameResources; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), m_material.size()));
	};
};

void InitDirect3DApp::BuildMaterials()
{
	auto pBricks0 = std::make_unique<Material>();
	pBricks0->Name = "bricks0";
	pBricks0->MatCBIndex = 0;
	pBricks0->DiffuseSrvHeapIndex = 0;
	pBricks0->DiffuseAlbedo = XMFLOAT4(Colors::ForestGreen);
	pBricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	pBricks0->Roughness = 0.3f;

	auto pStone0 = std::make_unique<Material>();
	pStone0->Name = "stone0";
	pStone0->MatCBIndex = 1;
	pStone0->DiffuseSrvHeapIndex = 1;
	pStone0->DiffuseAlbedo = XMFLOAT4(Colors::LightSteelBlue);
	pStone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	pStone0->Roughness = 0.3f;

	auto pTile0 = std::make_unique<Material>();
	pTile0->Name = "tile0";
	pTile0->MatCBIndex = 2;
	pTile0->DiffuseSrvHeapIndex = 2;
	pTile0->DiffuseAlbedo = XMFLOAT4(Colors::LightGray);
	pTile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	pTile0->Roughness = 0.2f;

	auto pSkullMat = std::make_unique<Material>();
	pSkullMat->Name = "skullMat";
	pSkullMat->MatCBIndex = 3;
	pSkullMat->DiffuseSrvHeapIndex = 3;
	pSkullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pSkullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05);
	pSkullMat->Roughness = 0.3f;

	m_material["bricks0"] = std::move(pBricks0);
	m_material["stone0"] = std::move(pStone0);
	m_material["tile0"] = std::move(pTile0);
	m_material["skullMat"] = std::move(pSkullMat);
};

void InitDirect3DApp::BuildRenderItems()
{
	auto boxRenderItem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRenderItem->m_worldMatrix, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	XMStoreFloat4x4(&boxRenderItem->m_textureTransformMatrix, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	boxRenderItem->m_uiObjectConstantBufferIndex = 0;
	boxRenderItem->m_pxMaterial = m_material["stone0"].get();
	boxRenderItem->m_pxGeometry = m_geometries["shapeGeo"].get();
	boxRenderItem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRenderItem->m_uiIndexCount = boxRenderItem->m_pxGeometry->DrawArgs["box"].m_uiIndexCount;
	boxRenderItem->m_uiStartIndexLocation = boxRenderItem->m_pxGeometry->DrawArgs["box"].m_uiStartIndexLocation;
	boxRenderItem->m_iBaseVertexLocation = boxRenderItem->m_pxGeometry->DrawArgs["box"].m_iBaseVertexLocation;
	m_allRenderItems.push_back(std::move(boxRenderItem));

	auto gridRenderItem = std::make_unique<RenderItem>();
	gridRenderItem->m_worldMatrix = MathHelper::Identity4x4();
	XMStoreFloat4x4(&boxRenderItem->m_textureTransformMatrix, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	gridRenderItem->m_uiObjectConstantBufferIndex = 1;
	gridRenderItem->m_pxMaterial = m_material["tile0"].get();
	gridRenderItem->m_pxGeometry = m_geometries["shapeGeo"].get();
	gridRenderItem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->m_uiIndexCount = gridRenderItem->m_pxGeometry->DrawArgs["grid"].m_uiIndexCount;
	gridRenderItem->m_uiStartIndexLocation = gridRenderItem->m_pxGeometry->DrawArgs["grid"].m_uiStartIndexLocation;
	gridRenderItem->m_iBaseVertexLocation = gridRenderItem->m_pxGeometry->DrawArgs["grid"].m_iBaseVertexLocation;
	m_allRenderItems.push_back(std::move(gridRenderItem));

	auto skullRenderItem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skullRenderItem->m_worldMatrix, XMMatrixScaling(0.5f, 0.5f, 0.5f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	skullRenderItem->m_textureTransformMatrix = MathHelper::Identity4x4();
	skullRenderItem->m_uiObjectConstantBufferIndex= 2;
	skullRenderItem->m_pxMaterial = m_material["skullMat"].get();
	skullRenderItem->m_pxGeometry = m_geometries["skullGeo"].get();
	skullRenderItem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullRenderItem->m_uiIndexCount = skullRenderItem->m_pxGeometry->DrawArgs["skull"].m_uiIndexCount;
	skullRenderItem->m_uiStartIndexLocation = skullRenderItem->m_pxGeometry->DrawArgs["skull"].m_uiStartIndexLocation;
	skullRenderItem->m_iBaseVertexLocation = skullRenderItem->m_pxGeometry->DrawArgs["skull"].m_iBaseVertexLocation;
	m_allRenderItems.push_back(std::move(skullRenderItem));
	XMMATRIX brickTexTransform = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	UINT uiObjectConstantBufferIndex = 3;
	for (int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = std::make_unique<RenderItem>();
		auto rightCylRitem = std::make_unique<RenderItem>();
		auto leftSphereRitem = std::make_unique<RenderItem>();
		auto rightSphereRitem = std::make_unique<RenderItem>();

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f);

		XMStoreFloat4x4(&leftCylRitem->m_worldMatrix, rightCylWorld);
		XMStoreFloat4x4(&leftCylRitem->m_textureTransformMatrix, brickTexTransform);
		leftCylRitem->m_uiObjectConstantBufferIndex = uiObjectConstantBufferIndex++;
		leftCylRitem->m_pxMaterial = m_material["bricks0"].get();
		leftCylRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		leftCylRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->m_uiIndexCount = leftCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_uiIndexCount;
		leftCylRitem->m_uiStartIndexLocation = leftCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_uiStartIndexLocation;
		leftCylRitem->m_iBaseVertexLocation = leftCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&rightCylRitem->m_worldMatrix, leftCylWorld);
		XMStoreFloat4x4(&rightCylRitem->m_textureTransformMatrix, brickTexTransform);
		rightCylRitem->m_uiObjectConstantBufferIndex = uiObjectConstantBufferIndex++;
		rightCylRitem->m_pxMaterial = m_material["bricks0"].get();
		rightCylRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		rightCylRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->m_uiIndexCount = rightCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_uiIndexCount;
		rightCylRitem->m_uiStartIndexLocation = rightCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_uiStartIndexLocation;
		rightCylRitem->m_iBaseVertexLocation = rightCylRitem->m_pxGeometry->DrawArgs["cylinder"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRitem->m_worldMatrix, leftSphereWorld);
		leftSphereRitem->m_textureTransformMatrix = MathHelper::Identity4x4();
		leftSphereRitem->m_uiObjectConstantBufferIndex = uiObjectConstantBufferIndex++;
		leftSphereRitem->m_pxMaterial = m_material["stone0"].get();
		leftSphereRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		leftSphereRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->m_uiIndexCount = leftSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_uiIndexCount;
		leftSphereRitem->m_uiStartIndexLocation = leftSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_uiStartIndexLocation;
		leftSphereRitem->m_iBaseVertexLocation = leftSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRitem->m_worldMatrix, rightSphereWorld);
		rightSphereRitem->m_textureTransformMatrix = MathHelper::Identity4x4();
		rightSphereRitem->m_uiObjectConstantBufferIndex = uiObjectConstantBufferIndex++;
		rightSphereRitem->m_pxMaterial = m_material["stone0"].get();
		rightSphereRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		rightSphereRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->m_uiIndexCount = rightSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_uiIndexCount;
		rightSphereRitem->m_uiStartIndexLocation = rightSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_uiStartIndexLocation;
		rightSphereRitem->m_iBaseVertexLocation = rightSphereRitem->m_pxGeometry->DrawArgs["sphere"].m_iBaseVertexLocation;

		m_allRenderItems.push_back(std::move(leftCylRitem));
		m_allRenderItems.push_back(std::move(rightCylRitem));
		m_allRenderItems.push_back(std::move(leftSphereRitem));
		m_allRenderItems.push_back(std::move(rightSphereRitem));
	};

	for (auto& e : m_allRenderItems)
	{
		m_opaqueRenderItems.push_back(e.get());
	};
};

void InitDirect3DApp::DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems)
{
	UINT uiObjectConstantBufferByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT uiMaterialConstantBufferByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectConstantBuffer = m_pxCurrentFrameResource->m_pxObjectConstantBuffer->Resource();
	auto materialConstantBuffer = m_pxCurrentFrameResource->m_pxMaterialConstantBuffer->Resource();

	for (size_t i = 0; i < kRenderItems.size(); ++i)
	{
		auto renderItem = kRenderItems[i];

		pGraphicsCommandList->IASetVertexBuffers(0, 1, &renderItem->m_pxGeometry->VertexBufferView());
		pGraphicsCommandList->IASetIndexBuffer(&renderItem->m_pxGeometry->IndexBufferView());
		pGraphicsCommandList->IASetPrimitiveTopology(renderItem->m_primitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objectConstantBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->m_uiObjectConstantBufferIndex * uiObjectConstantBufferByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS materialConstantBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->m_pxMaterial->MatCBIndex * uiMaterialConstantBufferByteSize;

		pGraphicsCommandList->SetGraphicsRootConstantBufferView(0, objectConstantBufferAddress);
		pGraphicsCommandList->SetGraphicsRootConstantBufferView(1, materialConstantBufferAddress);

		pGraphicsCommandList->DrawIndexedInstanced(renderItem->m_uiIndexCount, 1, renderItem->m_uiStartIndexLocation, renderItem->m_iBaseVertexLocation, 0);
	};
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int iShowCmd)
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InitDirect3DApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"Failed to initialize Direct3D 12!", MB_OK);
		return 0;
	};
};
