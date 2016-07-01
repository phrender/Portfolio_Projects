#include "D3DApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"
#include "Waves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

// Lightweight structure stores parameters to draw a shape. This will vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;

    XMFLOAT4X4 m_worldMatrix = MathHelper::Identity4x4();
	XMFLOAT4X4 m_textureTransformMatrix = MathHelper::Identity4x4();

	int m_iNumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT m_uiObjectConstantBufferIndex = -1;

	Material* m_pxMaterial = nullptr;
	MeshGeometry* m_pxGeometry = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT m_uiIndexCount = 0;
    UINT m_uiStartIndexLocation = 0;
    int m_iBaseVertexLocation = 0;
}; // !RenderItem

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	
	Count
}; // !RenderLayer

class InitDirect3DApp : public D3DApp
{
	public:
	    InitDirect3DApp(HINSTANCE hInstance);
	    InitDirect3DApp(const InitDirect3DApp& kInitDirect3DApp) = delete;
	    InitDirect3DApp& operator=(const InitDirect3DApp& kInitDirect3DApp) = delete;
	    ~InitDirect3DApp();
	
	    virtual bool Initialize()override;
	
	private:
	    virtual void OnResize()override;
	    virtual void Update(const GameTimer& kGameTimer) override;
	    virtual void Draw(const GameTimer& kGameTimer) override;
	
	    virtual void OnMouseDown(WPARAM btnState, int iPositionX, int iPositionY) override;
	    virtual void OnMouseUp(WPARAM btnState, int iPositionX, int iPositionY) override;
	    virtual void OnMouseMove(WPARAM btnState, int iPositionX, int iPositionY) override;
	
	    void OnKeyboardInput(const GameTimer& kGameTimer);
		void UpdateCamera(const GameTimer& kGameTimer);
		void AnimateMaterials(const GameTimer& kGameTimer);
		void UpdateObjectConstantBuffers(const GameTimer& kGameTimer);
		void UpdateMaterialConstantBuffers(const GameTimer& kGameTimer);
		void UpdateMainPassConstantBuffer(const GameTimer& kGameTimer);
		void UpdateWaves(const GameTimer& kGameTimer);
	
		void LoadTextures();
	    void BuildRootSignature();
		void BuildDescriptorHeaps();
	    void BuildShadersAndInputLayout();
	    void BuildShapeGeometry();
		void BuildSkullGeometry();
		void BuildLandGeometry();
		void BuildWavesGeometry();
		void BuildBoxGeometry();
	    void BuildPipelineStateObjects();
	    void BuildFrameResources();
	    void BuildMaterials();
	    void BuildRenderItems();
	    void DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems);

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

		float GetHillsHeight(float fX, float fZ) const;
		XMFLOAT3 GetHillsNormal(float fX, float fZ) const;
	 
	private:
		
	    ComPtr<ID3D12RootSignature> m_pxRootSignature;
		ComPtr<ID3D12DescriptorHeap> m_pxSrvDescriptorHeap;

		std::unique_ptr<Waves> m_pxWaves;
	
		FrameResource* m_pxCurrentFrameResource;
		RenderItem* m_pxWavesRenderItem;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_pipelineStateObjects;
		
		std::vector<std::unique_ptr<FrameResource>> m_frameResources;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
		std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
		std::vector<RenderItem*> m_opaqueRenderItems;
		std::vector<RenderItem*> m_renderItemLayer[(int)RenderLayer::Count];
	
	    PassConstants m_mainPassConstantBuffer;
	
		XMFLOAT4X4 m_viewMatrix;
		XMFLOAT4X4 m_projectionMatrix;
		XMFLOAT3 m_eyePosition;

		UINT m_uiCbvSrvDescriptorSize;
	
	    float m_fTheta;
	    float m_fPhi;
	    float m_fRadius;

		int m_iCurrentFrameResourceIndex;
	
	    POINT m_lastMousePosition;
}; // !InitDirect3DApp

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
	m_pxRootSignature = nullptr;
	m_pxSrvDescriptorHeap = nullptr;
	m_pxCurrentFrameResource = nullptr;

	m_pxWavesRenderItem = nullptr;

	m_viewMatrix = MathHelper::Identity4x4();
	m_projectionMatrix = MathHelper::Identity4x4();
	m_eyePosition = {0.0f, 0.0f, 0.0f};

	m_uiCbvSrvDescriptorSize = 0;

	m_fTheta = 1.5f * XM_PI;
	m_fPhi = 0.2f * XM_PI;
	m_fRadius = 15.0f;

	m_iCurrentFrameResourceIndex = 0;
};

InitDirect3DApp::~InitDirect3DApp()
{
	if (md3dDevice != nullptr)
		{ FlushCommandQueue(); };
};

bool InitDirect3DApp::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	m_uiCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pxWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.0f, 4.0f, 0.2f);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometry();
	BuildBoxGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPipelineStateObjects();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
};
 
void InitDirect3DApp::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&m_projectionMatrix, P);
};

void InitDirect3DApp::Update(const GameTimer& kGameTimer)
{
    OnKeyboardInput(kGameTimer);
	UpdateCamera(kGameTimer);

    // Cycle through the circular frame resource array.
	m_iCurrentFrameResourceIndex = (m_iCurrentFrameResourceIndex + 1) % gNumFrameResources;
	m_pxCurrentFrameResource = m_frameResources[m_iCurrentFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(m_pxCurrentFrameResource->m_ui64Fence != 0 && mFence->GetCompletedValue() < m_pxCurrentFrameResource->m_ui64Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(m_pxCurrentFrameResource->m_ui64Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
	};

	AnimateMaterials(kGameTimer);
	UpdateObjectConstantBuffers(kGameTimer);
	UpdateMaterialConstantBuffers(kGameTimer);
	UpdateMainPassConstantBuffer(kGameTimer);
	UpdateWaves(kGameTimer);
};

void InitDirect3DApp::Draw(const GameTimer& kGameTimer)
{
	auto cmdListAlloc = m_pxCurrentFrameResource->m_pxCommandListAllocator;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), m_pipelineStateObjects["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&m_mainPassConstantBuffer.m_fogColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* pDescriptorHeaps[] = { m_pxSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_pxRootSignature.Get());

	auto passCB = m_pxCurrentFrameResource->m_pxPassConstantBuffer->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), m_renderItemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(m_pipelineStateObjects["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), m_renderItemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(m_pipelineStateObjects["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), m_renderItemLayer[(int)RenderLayer::Transparent]);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_pxCurrentFrameResource->m_ui64Fence = ++mCurrentFence;


	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
};

void InitDirect3DApp::OnMouseDown(WPARAM btnState, int iPositionX, int iPositionY)
{
	m_lastMousePosition.x = iPositionX;
	m_lastMousePosition.y = iPositionY;

    SetCapture(mhMainWnd);
};

void InitDirect3DApp::OnMouseUp(WPARAM btnState, int iPositionX, int iPositionY)
{
    ReleaseCapture();
};

void InitDirect3DApp::OnMouseMove(WPARAM btnState, int iPositionX, int iPositionY)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(iPositionX - m_lastMousePosition.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(iPositionY - m_lastMousePosition.y));

        // Update angles based on input to orbit camera around box.
		m_fTheta += dx;
		m_fPhi += dy;

        // Restrict the angle mPhi.
		m_fPhi = MathHelper::Clamp(m_fPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.2 unit in the scene.
        float dx = 0.05f*static_cast<float>(iPositionX - m_lastMousePosition.x);
        float dy = 0.05f*static_cast<float>(iPositionY - m_lastMousePosition.y);

        // Update the camera radius based on input.
		m_fRadius += dx - dy;

        // Restrict the radius.
		m_fRadius = MathHelper::Clamp(m_fRadius, 5.0f, 150.0f);
	};

	m_lastMousePosition.x = iPositionX;
	m_lastMousePosition.y = iPositionY;
};
 
void InitDirect3DApp::OnKeyboardInput(const GameTimer& kGameTimer)
{
};
 
void InitDirect3DApp::UpdateCamera(const GameTimer& kGameTimer)
{
	// Convert Spherical to Cartesian coordinates.
	m_eyePosition.x = m_fRadius * sinf(m_fPhi) * cosf(m_fTheta);
	m_eyePosition.z = m_fRadius * sinf(m_fPhi) * sinf(m_fTheta);
	m_eyePosition.y = m_fRadius * cosf(m_fPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_eyePosition.x, m_eyePosition.y, m_eyePosition.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);
};

void InitDirect3DApp::AnimateMaterials(const GameTimer& kGameTimer)
{
	auto pWaterMaterial = m_materials["water"].get();

	float& fTU = pWaterMaterial->m_materialTransform(3, 0);
	float& fTV = pWaterMaterial->m_materialTransform(3, 1);

	fTU += 0.1f * kGameTimer.DeltaTime();
	fTV += 0.02f * kGameTimer.DeltaTime();

	if (fTU >= 1.0f)
		{ fTU -= 1.0f; };

	if (fTV >= 1.0f)
		{ fTV -= 1.0f; };

	pWaterMaterial->m_materialTransform(3, 0);
	pWaterMaterial->m_materialTransform(3, 1);

	// Material has changed, so need to update cbuffer
	pWaterMaterial->m_iNumFramesDirty = gNumFrameResources;
};

void InitDirect3DApp::UpdateObjectConstantBuffers(const GameTimer& kGameTimer)
{
	auto currObjectCB = m_pxCurrentFrameResource->m_pxObjectConstantBuffer.get();
	for(auto& e : m_allRenderItems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->m_iNumFramesDirty > 0)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&e->m_worldMatrix);
			XMMATRIX textureTransformMatrix = XMLoadFloat4x4(&e->m_textureTransformMatrix);

			ObjectConstants objectConstants;
			XMStoreFloat4x4(&objectConstants.m_worldMatrix, XMMatrixTranspose(worldMatrix));
			XMStoreFloat4x4(&objectConstants.m_textureTransformMatrix, XMMatrixTranspose(textureTransformMatrix));

			currObjectCB->CopyData(e->m_uiObjectConstantBufferIndex, objectConstants);

			// Next FrameResource need to be updated too.
			e->m_iNumFramesDirty--;
		};
	};
};

void InitDirect3DApp::UpdateMaterialConstantBuffers(const GameTimer& kGameTimer)
{
	auto pCurrentMaterialConstantBuffer = m_pxCurrentFrameResource->m_pxMaterialConstantBuffer.get();
	for(auto& e : m_materials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* pMaterial = e.second.get();
		if(pMaterial->m_iNumFramesDirty > 0)
		{
			XMMATRIX materialTransformMatrix = XMLoadFloat4x4(&pMaterial->m_materialTransform);

			MaterialConstants materialConstants;
			materialConstants.m_diffuseAlbedo = pMaterial->m_diffuseAlbedo;
			materialConstants.m_fresnelR0 = pMaterial->m_fresnelR0;
			materialConstants.m_fRoughness = pMaterial->m_fRoughness;
			XMStoreFloat4x4(&materialConstants.m_materialTransform, XMMatrixTranspose(materialTransformMatrix));

			pCurrentMaterialConstantBuffer->CopyData(pMaterial->m_iMaterialConstantBufferIndex, materialConstants);

			// Next FrameResource need to be updated too.
			pMaterial->m_iNumFramesDirty--;
		};
	};
};

void InitDirect3DApp::UpdateMainPassConstantBuffer(const GameTimer& kGameTimer)
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);

	XMMATRIX viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
	XMMATRIX inverseViewMatrix = XMMatrixInverse(&XMMatrixDeterminant(viewMatrix), viewMatrix);
	XMMATRIX inverseProjectionMatrix = XMMatrixInverse(&XMMatrixDeterminant(projectionMatrix), projectionMatrix);
	XMMATRIX inverseViewProjectionMatrix = XMMatrixInverse(&XMMatrixDeterminant(viewProjectionMatrix), viewProjectionMatrix);

	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_viewMatrix, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseViewMatrix, XMMatrixTranspose(inverseViewMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_projection, XMMatrixTranspose(projectionMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseProjectionMatrix, XMMatrixTranspose(inverseProjectionMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_viewProjectionMatrix, XMMatrixTranspose(viewProjectionMatrix));
	XMStoreFloat4x4(&m_mainPassConstantBuffer.m_inverseViewProjectionMatrix, XMMatrixTranspose(inverseViewProjectionMatrix));
	m_mainPassConstantBuffer.m_eyePosition = m_eyePosition;
	m_mainPassConstantBuffer.m_renderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	m_mainPassConstantBuffer.m_inverseRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	m_mainPassConstantBuffer.m_fNearZ = 1.0f;
	m_mainPassConstantBuffer.m_fFarZ = 1000.0f;
	m_mainPassConstantBuffer.m_fTotalTime = kGameTimer.TotalTime();
	m_mainPassConstantBuffer.m_fDeltaTime = kGameTimer.DeltaTime();
	m_mainPassConstantBuffer.m_ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_mainPassConstantBuffer.m_lights[0].m_direction = { 0.57735f, -0.57735f, 0.57735f };
	m_mainPassConstantBuffer.m_lights[0].m_strength = { 0.6f, 0.6f, 0.6f };
	m_mainPassConstantBuffer.m_lights[1].m_direction = { -0.57735f, -0.57735f, 0.57735f };
	m_mainPassConstantBuffer.m_lights[1].m_strength = { 0.3f, 0.3f, 0.3f };
	m_mainPassConstantBuffer.m_lights[2].m_direction = { 0.0f, -0.707f, -0.707f };
	m_mainPassConstantBuffer.m_lights[2].m_strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_pxCurrentFrameResource->m_pxPassConstantBuffer.get();
	currPassCB->CopyData(0, m_mainPassConstantBuffer);
};

void InitDirect3DApp::UpdateWaves(const GameTimer& kGameTimer)
{
	static float kfTimerBase = 0.0f;

	if ((mTimer.TotalTime() - kfTimerBase) >= 0.25f)
	{
		kfTimerBase += 0.25f;

		int i = MathHelper::Rand(4, m_pxWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, m_pxWaves->ColumnCount() - 5);

		float fRand = MathHelper::RandF(0.2f, 0.5f);

		m_pxWaves->Disturb(i, j, fRand);
	};

	m_pxWaves->Update(kGameTimer.DeltaTime());

	auto pCurrentWavesVertexBuffer = m_pxCurrentFrameResource->m_pxWavesVertexBuffer.get();
	for (int i = 0; i < m_pxWaves->VertexCount(); ++i)
	{
		Vertex vertex;

		vertex.m_position = m_pxWaves->Position(i);
		vertex.m_normal = m_pxWaves->Normal(i);

		vertex.m_textureCoord.x = 0.5f + vertex.m_position.x / m_pxWaves->Width();
		vertex.m_textureCoord.y = 0.5f - vertex.m_position.z / m_pxWaves->Depth();

		pCurrentWavesVertexBuffer->CopyData(i, vertex);
	};

	m_pxWavesRenderItem->m_pxGeometry->m_pxVertexBufferGPU = pCurrentWavesVertexBuffer->Resource();
};

void InitDirect3DApp::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->m_strName = "grassTex";
	grassTex->m_wstrFilename = L"Data/Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), grassTex->m_wstrFilename.c_str(), grassTex->m_pxResource, grassTex->m_pxUploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->m_strName = "waterTex";
	waterTex->m_wstrFilename = L"Data/Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), waterTex->m_wstrFilename.c_str(), waterTex->m_pxResource, waterTex->m_pxUploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->m_strName = "fenceTex";
	fenceTex->m_wstrFilename = L"Data/Textures/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), fenceTex->m_wstrFilename.c_str(), fenceTex->m_pxResource, fenceTex->m_pxUploadHeap)); 
	
	m_textures[grassTex->m_strName] = std::move(grassTex);
	m_textures[waterTex->m_strName] = std::move(waterTex);
	m_textures[fenceTex->m_strName] = std::move(fenceTex);
};

void InitDirect3DApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE textureTableDesc;
	textureTableDesc.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &textureTableDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> pSerializedRootSignature = nullptr;
	ComPtr<ID3DBlob> pErrorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, pSerializedRootSignature.GetAddressOf(), pErrorBlob.GetAddressOf());

	if (pErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, pSerializedRootSignature->GetBufferPointer(), pSerializedRootSignature->GetBufferSize(), IID_PPV_ARGS(m_pxRootSignature.GetAddressOf())));
};

void InitDirect3DApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC shaderResourceViewHeapDesc = {};
	shaderResourceViewHeapDesc.NumDescriptors = 3;
	shaderResourceViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	shaderResourceViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&shaderResourceViewHeapDesc, IID_PPV_ARGS(&m_pxSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_pxSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = m_textures["grassTex"]->m_pxResource;
	auto waterTex = m_textures["waterTex"]->m_pxResource;
	auto fenceTex = m_textures["fenceTex"]->m_pxResource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_uiCbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_uiCbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);
};

void InitDirect3DApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] = { "FOG", "1", NULL, NULL };

	const D3D_SHADER_MACRO alphaTestDefines[] = { "FOG", "1", "ALPHA_TEST", "1", NULL, NULL };

	m_shaders["standardVS"] = d3dUtil::CompileShader(L"Data\\Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
	m_shaders["opaquePS"] = d3dUtil::CompileShader(L"Data\\Shaders\\Default.hlsl", defines, "PS", "ps_5_0");
	m_shaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Data\\Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_0");

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
};

void InitDirect3DApp::BuildLandGeometry()
{

	GeometryGenerator geometryGenerator;
	GeometryGenerator::MeshData grid = geometryGenerator.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex> vertices(grid.m_vertices.size());
	for (size_t i = 0; i < grid.m_vertices.size(); ++i)
	{
		auto& p = grid.m_vertices[i].m_position;
		vertices[i].m_position = p;
		vertices[i].m_position.y = GetHillsHeight(p.x, p.z);
		vertices[i].m_normal = GetHillsNormal(p.x, p.z);
		vertices[i].m_textureCoord = grid.m_vertices[i].m_textureCoord;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->m_strName = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->m_pxVertexBufferCPU));
	CopyMemory(geo->m_pxVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->m_pxIndexBufferCPU));
	CopyMemory(geo->m_pxIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->m_pxVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->m_pxVertexBufferUploader);

	geo->m_pxIndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->m_pxIndexBufferUploader);

	geo->m_uiVertexByteStride = sizeof(Vertex);
	geo->m_uiVertexBufferByteSize = vbByteSize;
	geo->m_indexFormat = DXGI_FORMAT_R16_UINT;
	geo->m_uiIndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.m_uiIndexCount = (UINT)indices.size();
	submesh.m_uiStartIndexLocation = 0;
	submesh.m_iBaseVertexLocation = 0;

	geo->m_drawArgs["grid"] = submesh;

	m_geometries["landGeo"] = std::move(geo);

};

void InitDirect3DApp::BuildWavesGeometry()
{
	std::vector<std::uint16_t> indices(3 * m_pxWaves->TriangleCount()); // 3 indices per face
	assert(m_pxWaves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = m_pxWaves->RowCount();
	int n = m_pxWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = m_pxWaves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->m_strName = "waterGeo";

	// Set dynamically.
	geo->m_pxVertexBufferCPU = nullptr;
	geo->m_pxVertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->m_pxIndexBufferCPU));
	CopyMemory(geo->m_pxIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->m_pxIndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->m_pxIndexBufferUploader);

	geo->m_uiVertexByteStride = sizeof(Vertex);
	geo->m_uiVertexBufferByteSize = vbByteSize;
	geo->m_indexFormat = DXGI_FORMAT_R16_UINT;
	geo->m_uiIndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.m_uiIndexCount = (UINT)indices.size();
	submesh.m_uiStartIndexLocation = 0;
	submesh.m_iBaseVertexLocation = 0;

	geo->m_drawArgs["grid"] = submesh;

	m_geometries["waterGeo"] = std::move(geo);
};

void InitDirect3DApp::BuildBoxGeometry()
{
	GeometryGenerator geometryGenerator;
	GeometryGenerator::MeshData box = geometryGenerator.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.m_vertices.size());
	for (size_t i = 0; i < box.m_vertices.size(); ++i)
	{
		auto& p = box.m_vertices[i].m_position;
		vertices[i].m_position = p;
		vertices[i].m_normal = box.m_vertices[i].m_normal;
		vertices[i].m_textureCoord = box.m_vertices[i].m_textureCoord;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->m_strName = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->m_pxVertexBufferCPU));
	CopyMemory(geo->m_pxVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->m_pxIndexBufferCPU));
	CopyMemory(geo->m_pxIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->m_pxVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->m_pxVertexBufferUploader);

	geo->m_pxIndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->m_pxIndexBufferUploader);

	geo->m_uiVertexByteStride = sizeof(Vertex);
	geo->m_uiVertexBufferByteSize = vbByteSize;
	geo->m_indexFormat = DXGI_FORMAT_R16_UINT;
	geo->m_uiIndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.m_uiIndexCount = (UINT)indices.size();
	submesh.m_uiStartIndexLocation = 0;
	submesh.m_iBaseVertexLocation = 0;

	geo->m_drawArgs["box"] = submesh;

	m_geometries["boxGeo"] = std::move(geo);
};

void InitDirect3DApp::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.m_vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.m_vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.m_vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
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

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount = box.m_vertices.size() + grid.m_vertices.size() + sphere.m_vertices.size() + cylinder.m_vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = box.m_vertices[i].m_position;
		vertices[k].m_normal = box.m_vertices[i].m_normal;
		vertices[k].m_textureCoord = box.m_vertices[i].m_textureCoord;
	}

	for (size_t i = 0; i < grid.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = grid.m_vertices[i].m_position;
		vertices[k].m_normal = grid.m_vertices[i].m_normal;
		vertices[k].m_textureCoord = grid.m_vertices[i].m_textureCoord;
	}

	for (size_t i = 0; i < sphere.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = sphere.m_vertices[i].m_position;
		vertices[k].m_normal = sphere.m_vertices[i].m_normal;
		vertices[k].m_textureCoord = sphere.m_vertices[i].m_textureCoord;
	}

	for (size_t i = 0; i < cylinder.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_position = cylinder.m_vertices[i].m_position;
		vertices[k].m_normal = cylinder.m_vertices[i].m_normal;
		vertices[k].m_textureCoord = cylinder.m_vertices[i].m_textureCoord;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->m_strName = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->m_pxVertexBufferCPU));
	CopyMemory(geo->m_pxVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->m_pxIndexBufferCPU));
	CopyMemory(geo->m_pxIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->m_pxVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->m_pxVertexBufferUploader);

	geo->m_pxIndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->m_pxIndexBufferUploader);

	geo->m_uiVertexByteStride = sizeof(Vertex);
	geo->m_uiVertexBufferByteSize = vbByteSize;
	geo->m_indexFormat = DXGI_FORMAT_R16_UINT;
	geo->m_uiIndexBufferByteSize = ibByteSize;

	geo->m_drawArgs["box"] = boxSubmesh;
	geo->m_drawArgs["grid"] = gridSubmesh;
	geo->m_drawArgs["sphere"] = sphereSubmesh;
	geo->m_drawArgs["cylinder"] = cylinderSubmesh;

	m_geometries[geo->m_strName] = std::move(geo);
};

void InitDirect3DApp::BuildSkullGeometry()
{
	std::ifstream fin("Models/skull.txt");

	if(!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	};

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].m_position.x >> vertices[i].m_position.y >> vertices[i].m_position.z;
		fin >> vertices[i].m_normal.x >> vertices[i].m_normal.y >> vertices[i].m_normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	};

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT kuiVertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT kuiIndexBufferByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto pGeometry = std::make_unique<MeshGeometry>();
	pGeometry->m_strName = "skullGeo";

	ThrowIfFailed(D3DCreateBlob(kuiVertexBufferByteSize, &pGeometry->m_pxVertexBufferCPU));
	CopyMemory(pGeometry->m_pxVertexBufferCPU->GetBufferPointer(), vertices.data(), kuiVertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(kuiIndexBufferByteSize, &pGeometry->m_pxIndexBufferCPU));
	CopyMemory(pGeometry->m_pxIndexBufferCPU->GetBufferPointer(), indices.data(), kuiIndexBufferByteSize);

	pGeometry->m_pxVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), kuiVertexBufferByteSize, pGeometry->m_pxVertexBufferUploader);

	pGeometry->m_pxIndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), kuiIndexBufferByteSize, pGeometry->m_pxIndexBufferUploader);

	pGeometry->m_uiVertexByteStride = sizeof(Vertex);
	pGeometry->m_uiVertexBufferByteSize = kuiVertexBufferByteSize;
	pGeometry->m_indexFormat = DXGI_FORMAT_R32_UINT;
	pGeometry->m_uiIndexBufferByteSize = kuiIndexBufferByteSize;

	SubmeshGeometry submesh;
	submesh.m_uiIndexCount = (UINT)indices.size();
	submesh.m_uiStartIndexLocation = 0;
	submesh.m_iBaseVertexLocation = 0;

	pGeometry->m_drawArgs["skull"] = submesh;

	m_geometries[pGeometry->m_strName] = std::move(pGeometry);
};

void InitDirect3DApp::BuildPipelineStateObjects()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
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
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_pipelineStateObjects["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_pipelineStateObjects["transparent"])));

	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["alphaTestedPS"]->GetBufferPointer()),
		m_shaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_pipelineStateObjects["alphaTested"])));
};

void InitDirect3DApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), (UINT)m_materials.size()));
	};
};

void InitDirect3DApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->m_strName = "grass";
	grass->m_iMaterialConstantBufferIndex = 0;
	grass->m_iDiffuseSrvHeapIndex = 0;
	grass->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->m_fresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->m_fRoughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->m_strName = "water";
	water->m_iMaterialConstantBufferIndex = 1;
	water->m_iDiffuseSrvHeapIndex = 1;
	water->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->m_fresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->m_fRoughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->m_strName = "wirefence";
	wirefence->m_iMaterialConstantBufferIndex = 2;
	wirefence->m_iDiffuseSrvHeapIndex = 2;
	wirefence->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->m_fresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence->m_fRoughness = 0.25f;

	m_materials["grass"] = std::move(grass);
	m_materials["water"] = std::move(water);
	m_materials["wirefence"] = std::move(wirefence);
};

void InitDirect3DApp::BuildRenderItems()
{
	auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->m_worldMatrix = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesRitem->m_textureTransformMatrix, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->m_uiObjectConstantBufferIndex = 0;
	wavesRitem->m_pxMaterial = m_materials["water"].get();
	wavesRitem->m_pxGeometry = m_geometries["waterGeo"].get();
	wavesRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->m_uiIndexCount = wavesRitem->m_pxGeometry->m_drawArgs["grid"].m_uiIndexCount;
	wavesRitem->m_uiStartIndexLocation = wavesRitem->m_pxGeometry->m_drawArgs["grid"].m_uiStartIndexLocation;
	wavesRitem->m_iBaseVertexLocation = wavesRitem->m_pxGeometry->m_drawArgs["grid"].m_iBaseVertexLocation;

	m_pxWavesRenderItem = wavesRitem.get();

	m_renderItemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->m_worldMatrix = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->m_textureTransformMatrix, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->m_uiObjectConstantBufferIndex = 1;
	gridRitem->m_pxMaterial = m_materials["grass"].get();
	gridRitem->m_pxGeometry = m_geometries["landGeo"].get();
	gridRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->m_uiIndexCount = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_uiIndexCount;
	gridRitem->m_uiStartIndexLocation = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_uiStartIndexLocation;
	gridRitem->m_iBaseVertexLocation = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_iBaseVertexLocation;

	m_renderItemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->m_worldMatrix, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRitem->m_uiObjectConstantBufferIndex = 2;
	boxRitem->m_pxMaterial = m_materials["wirefence"].get();
	boxRitem->m_pxGeometry = m_geometries["boxGeo"].get();
	boxRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->m_uiIndexCount = boxRitem->m_pxGeometry->m_drawArgs["box"].m_uiIndexCount;
	boxRitem->m_uiStartIndexLocation = boxRitem->m_pxGeometry->m_drawArgs["box"].m_uiStartIndexLocation;
	boxRitem->m_iBaseVertexLocation = boxRitem->m_pxGeometry->m_drawArgs["box"].m_iBaseVertexLocation;

	m_renderItemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem.get());

	m_allRenderItems.push_back(std::move(wavesRitem));
	m_allRenderItems.push_back(std::move(gridRitem));
	m_allRenderItems.push_back(std::move(boxRitem));
}

void InitDirect3DApp::DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pxCurrentFrameResource->m_pxObjectConstantBuffer->Resource();
	auto matCB = m_pxCurrentFrameResource->m_pxMaterialConstantBuffer->Resource();

	// For each render item...
	for (size_t i = 0; i < kRenderItems.size(); ++i)
	{
		auto renderItem = kRenderItems[i];

		pGraphicsCommandList->IASetVertexBuffers(0, 1, &renderItem->m_pxGeometry->VertexBufferView());
		pGraphicsCommandList->IASetIndexBuffer(&renderItem->m_pxGeometry->IndexBufferView());
		pGraphicsCommandList->IASetPrimitiveTopology(renderItem->m_primitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_pxSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(renderItem->m_pxMaterial->m_iDiffuseSrvHeapIndex, m_uiCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + renderItem->m_uiObjectConstantBufferIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + renderItem->m_pxMaterial->m_iMaterialConstantBufferIndex * matCBByteSize;

		pGraphicsCommandList->SetGraphicsRootDescriptorTable(0, tex);
		pGraphicsCommandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		pGraphicsCommandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		pGraphicsCommandList->DrawIndexedInstanced(renderItem->m_uiIndexCount, 1, renderItem->m_uiStartIndexLocation, renderItem->m_iBaseVertexLocation, 0);
	}
};

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> InitDirect3DApp::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC kPointWrap(
		0,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);	// addressW

	const CD3DX12_STATIC_SAMPLER_DESC kPointClamp(
		1,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	// addressW

	const CD3DX12_STATIC_SAMPLER_DESC kLinearWrap(
		2,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);	// addressW

	const CD3DX12_STATIC_SAMPLER_DESC kLinearClamp(
		3,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	// addressW

	const CD3DX12_STATIC_SAMPLER_DESC kAnisotropicWrap(
		4,									// shaderRegister
		D3D12_FILTER_ANISOTROPIC,			// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressW
		0.0f,								// mipLODBias
		8);									// maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC kAnisotropicClamp(
		5,									// shaderRegister
		D3D12_FILTER_ANISOTROPIC,			// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressW
		0.0f,								// mipLODBias
		8);									// maxAnisotropy

	return { kPointWrap, kPointClamp, kLinearWrap, kLinearClamp, kAnisotropicWrap, kAnisotropicClamp };
};

float InitDirect3DApp::GetHillsHeight(float fX, float fZ) const
{
	return 0.3f * (fZ * sinf(0.1f * fX) + fX * cosf(0.1f * fZ));
};

XMFLOAT3 InitDirect3DApp::GetHillsNormal(float fX, float fZ) const
{
	XMFLOAT3 normal
			(
				-0.03f * fZ * cosf(0.1f * fX) - 0.3f * cosf(0.1f * fZ),
				1.0f,
				-0.3f * sinf(0.1f * fX) + 0.03f * fX * sinf(0.1f * fZ)
			);

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&normal));
	XMStoreFloat3(&normal, unitNormal);

	return normal;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
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
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	};
};
