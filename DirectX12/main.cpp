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
};

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
	
		void LoadTextures();
	    void BuildRootSignature();
		void BuildDescriptorHeaps();
	    void BuildShadersAndInputLayout();
	    void BuildShapeGeometry();
		void BuildSkullGeometry();
	    void BuildPipelineStateObjects();
	    void BuildFrameResources();
	    void BuildMaterials();
	    void BuildRenderItems();
	    void DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems);

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	 
	private:
		
	    ComPtr<ID3D12RootSignature> m_pxRootSignature;
		ComPtr<ID3D12DescriptorHeap> m_pxSrvDescriptorHeap;
		//ComPtr<ID3D12PipelineState> m_pxOpaquePipelineStateObject;
	
		FrameResource* m_pxCurrentFrameResource;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_pipelineStateObjects;
		
		std::vector<std::unique_ptr<FrameResource>> m_frameResources;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
		std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
		std::vector<RenderItem*> m_opaqueRenderItems;
	
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
};

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
	m_pxRootSignature = nullptr;
	m_pxSrvDescriptorHeap = nullptr;
	//m_pxOpaquePipelineStateObject = nullptr;
	m_pxCurrentFrameResource = nullptr;

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

	LoadTextures();
    BuildRootSignature();
	BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildShapeGeometry();
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
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_pxSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_pxRootSignature.Get());

	auto passCB = m_pxCurrentFrameResource->m_pxPassConstantBuffer->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), m_opaqueRenderItems);

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

void InitDirect3DApp::LoadTextures()
{
	auto pBricksTexture = std::make_unique<Texture>();
	pBricksTexture->m_strName = "bricksTexture";
	pBricksTexture->m_wstrFilename = L"Textures/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), pBricksTexture->m_wstrFilename.c_str(), pBricksTexture->m_pxResource, pBricksTexture->m_pxUploadHeap));

	auto pStoneTexture = std::make_unique<Texture>();
	pStoneTexture->m_strName = "stoneTexture";
	pStoneTexture->m_wstrFilename = L"Textures/stone.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), pStoneTexture->m_wstrFilename.c_str(), pStoneTexture->m_pxResource, pStoneTexture->m_pxUploadHeap));

	auto pTileTexture = std::make_unique<Texture>();
	pTileTexture->m_strName = "tileTexture";
	pTileTexture->m_wstrFilename = L"Textures/stone.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), pTileTexture->m_wstrFilename.c_str(), pTileTexture->m_pxResource, pTileTexture->m_pxUploadHeap));

	m_textures[pBricksTexture->m_strName] = std::move(pBricksTexture);
	m_textures[pStoneTexture->m_strName] = std::move(pStoneTexture);
	m_textures[pTileTexture->m_strName] = std::move(pTileTexture);
};

void InitDirect3DApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE textureTable;
	textureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create root CBV.
	slotRootParameter[0].InitAsDescriptorTable(1, &textureTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> pSerializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, pSerializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
		{ OutputDebugStringA((char*)errorBlob->GetBufferPointer()); }
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(), pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_pxRootSignature.GetAddressOf())));
};

void InitDirect3DApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC shaderResourceViewHeapDesc;
	ZeroMemory(&shaderResourceViewHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	shaderResourceViewHeapDesc.NumDescriptors = 3;
	shaderResourceViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	shaderResourceViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&shaderResourceViewHeapDesc, IID_PPV_ARGS(&m_pxSrvDescriptorHeap)));
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_pxSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = m_textures["bricksTexture"]->m_pxResource;
	auto stoneTex = m_textures["stoneTexture"]->m_pxResource;
	auto tileTex = m_textures["tileTexture"]->m_pxResource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_uiCbvSrvDescriptorSize);

	srvDesc.Format = stoneTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(stoneTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_uiCbvSrvDescriptorSize);

	srvDesc.Format = tileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);
};

void InitDirect3DApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] = { "ALPHA_TEST", "1", NULL, NULL };

	m_shaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	
	m_inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePipelineStateOnjectDesc;

	ZeroMemory(&opaquePipelineStateOnjectDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePipelineStateOnjectDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	opaquePipelineStateOnjectDesc.pRootSignature = m_pxRootSignature.Get();
	opaquePipelineStateOnjectDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	opaquePipelineStateOnjectDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};
	opaquePipelineStateOnjectDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePipelineStateOnjectDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePipelineStateOnjectDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePipelineStateOnjectDesc.SampleMask = UINT_MAX;
	opaquePipelineStateOnjectDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePipelineStateOnjectDesc.NumRenderTargets = 1;
	opaquePipelineStateOnjectDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePipelineStateOnjectDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePipelineStateOnjectDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePipelineStateOnjectDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePipelineStateOnjectDesc, IID_PPV_ARGS(&m_pipelineStateObjects["opaque"])));
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
	auto bricks0 = std::make_unique<Material>();
	bricks0->m_strName = "bricks0";
	bricks0->m_iMaterialConstantBufferIndex = 0;
	bricks0->m_iDiffuseSrvHeapIndex = 0;
	bricks0->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->m_fresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->m_fRoughness = 0.1f;

	auto stone0 = std::make_unique<Material>();
	stone0->m_strName = "stone0";
	stone0->m_iMaterialConstantBufferIndex = 1;
	stone0->m_iDiffuseSrvHeapIndex = 1;
	stone0->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone0->m_fresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->m_fRoughness = 0.3f;

	auto tile0 = std::make_unique<Material>();
	tile0->m_strName = "tile0";
	tile0->m_iMaterialConstantBufferIndex = 2;
	tile0->m_iDiffuseSrvHeapIndex = 2;
	tile0->m_diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile0->m_fresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->m_fRoughness = 0.3f;

	m_materials["bricks0"] = std::move(bricks0);
	m_materials["stone0"] = std::move(stone0);
	m_materials["tile0"] = std::move(tile0);
};

void InitDirect3DApp::BuildRenderItems()
{
	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->m_worldMatrix, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	XMStoreFloat4x4(&boxRitem->m_textureTransformMatrix, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	boxRitem->m_uiObjectConstantBufferIndex = 0;
	boxRitem->m_pxMaterial = m_materials["stone0"].get();
	boxRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
	boxRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->m_uiIndexCount = boxRitem->m_pxGeometry->m_drawArgs["box"].m_uiIndexCount;
	boxRitem->m_uiStartIndexLocation = boxRitem->m_pxGeometry->m_drawArgs["box"].m_uiStartIndexLocation;
	boxRitem->m_iBaseVertexLocation = boxRitem->m_pxGeometry->m_drawArgs["box"].m_iBaseVertexLocation;
	m_allRenderItems.push_back(std::move(boxRitem));

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->m_worldMatrix = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->m_textureTransformMatrix, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	gridRitem->m_uiObjectConstantBufferIndex = 1;
	gridRitem->m_pxMaterial = m_materials["tile0"].get();
	gridRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
	gridRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->m_uiIndexCount = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_uiIndexCount;
	gridRitem->m_uiStartIndexLocation = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_uiStartIndexLocation;
	gridRitem->m_iBaseVertexLocation = gridRitem->m_pxGeometry->m_drawArgs["grid"].m_iBaseVertexLocation;
	m_allRenderItems.push_back(std::move(gridRitem));

	XMMATRIX brickTexTransform = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	UINT objCBIndex = 2;
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
		leftCylRitem->m_uiObjectConstantBufferIndex = objCBIndex++;
		leftCylRitem->m_pxMaterial = m_materials["bricks0"].get();
		leftCylRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		leftCylRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->m_uiIndexCount = leftCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_uiIndexCount;
		leftCylRitem->m_uiStartIndexLocation = leftCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_uiStartIndexLocation;
		leftCylRitem->m_iBaseVertexLocation = leftCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&rightCylRitem->m_worldMatrix, leftCylWorld);
		XMStoreFloat4x4(&rightCylRitem->m_textureTransformMatrix, brickTexTransform);
		rightCylRitem->m_uiObjectConstantBufferIndex = objCBIndex++;
		rightCylRitem->m_pxMaterial = m_materials["bricks0"].get();
		rightCylRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		rightCylRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->m_uiIndexCount = rightCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_uiIndexCount;
		rightCylRitem->m_uiStartIndexLocation = rightCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_uiStartIndexLocation;
		rightCylRitem->m_iBaseVertexLocation = rightCylRitem->m_pxGeometry->m_drawArgs["cylinder"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRitem->m_worldMatrix, leftSphereWorld);
		leftSphereRitem->m_textureTransformMatrix = MathHelper::Identity4x4();
		leftSphereRitem->m_uiObjectConstantBufferIndex = objCBIndex++;
		leftSphereRitem->m_pxMaterial = m_materials["stone0"].get();
		leftSphereRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		leftSphereRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->m_uiIndexCount = leftSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_uiIndexCount;
		leftSphereRitem->m_uiStartIndexLocation = leftSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_uiStartIndexLocation;
		leftSphereRitem->m_iBaseVertexLocation = leftSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_iBaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRitem->m_worldMatrix, rightSphereWorld);
		rightSphereRitem->m_textureTransformMatrix = MathHelper::Identity4x4();
		rightSphereRitem->m_uiObjectConstantBufferIndex = objCBIndex++;
		rightSphereRitem->m_pxMaterial = m_materials["stone0"].get();
		rightSphereRitem->m_pxGeometry = m_geometries["shapeGeo"].get();
		rightSphereRitem->m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->m_uiIndexCount = rightSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_uiIndexCount;
		rightSphereRitem->m_uiStartIndexLocation = rightSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_uiStartIndexLocation;
		rightSphereRitem->m_iBaseVertexLocation = rightSphereRitem->m_pxGeometry->m_drawArgs["sphere"].m_iBaseVertexLocation;

		m_allRenderItems.push_back(std::move(leftCylRitem));
		m_allRenderItems.push_back(std::move(rightCylRitem));
		m_allRenderItems.push_back(std::move(leftSphereRitem));
		m_allRenderItems.push_back(std::move(rightSphereRitem));
	}

	// All the render items are opaque.
	for (auto& e : m_allRenderItems)
	{
		m_opaqueRenderItems.push_back(e.get());
	};
}

void InitDirect3DApp::DrawRenderItems(ID3D12GraphicsCommandList* pGraphicsCommandList, const std::vector<RenderItem*>& kRenderItems)
{
	UINT uiObjConstantBufferByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT uiMaterialConstantBufferByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto pObjectConstantBuffer = m_pxCurrentFrameResource->m_pxObjectConstantBuffer->Resource();
	auto pMaterialConstantBuffer = m_pxCurrentFrameResource->m_pxMaterialConstantBuffer->Resource();

	// For each render item...
	for (size_t i = 0; i < kRenderItems.size(); ++i)
	{
		auto renderItem = kRenderItems[i];

		pGraphicsCommandList->IASetVertexBuffers(0, 1, &renderItem->m_pxGeometry->VertexBufferView());
		pGraphicsCommandList->IASetIndexBuffer(&renderItem->m_pxGeometry->IndexBufferView());
		pGraphicsCommandList->IASetPrimitiveTopology(renderItem->m_primitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE textureHandler(m_pxSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		textureHandler.Offset(renderItem->m_pxMaterial->m_iDiffuseSrvHeapIndex, m_uiCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objectConstantBufferAddress = pObjectConstantBuffer->GetGPUVirtualAddress() + renderItem->m_uiObjectConstantBufferIndex * uiObjConstantBufferByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS materialConstantBufferAddress = pMaterialConstantBuffer->GetGPUVirtualAddress() + renderItem->m_pxMaterial->m_iMaterialConstantBufferIndex * uiMaterialConstantBufferByteSize;

		pGraphicsCommandList->SetGraphicsRootDescriptorTable(0, textureHandler);
		pGraphicsCommandList->SetGraphicsRootConstantBufferView(1, objectConstantBufferAddress);
		pGraphicsCommandList->SetGraphicsRootConstantBufferView(3, materialConstantBufferAddress);

		pGraphicsCommandList->DrawIndexedInstanced(renderItem->m_uiIndexCount, 1, renderItem->m_uiStartIndexLocation, renderItem->m_iBaseVertexLocation, 0);
	};
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
