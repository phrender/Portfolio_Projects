#include "D3DApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 m_position;
	XMFLOAT4 m_color;
}; // Vertex

struct ObjectConstants
{
	XMFLOAT4X4 m_worldViewProjection = MathHelper::Identity4x4();
}; // ObjectConstants

class InitDirect3DApp : public D3DApp
{

	public:

		InitDirect3DApp(HINSTANCE hInstance);
		InitDirect3DApp(const InitDirect3DApp& kDirect3DApp) = delete;
		InitDirect3DApp& operator=(const InitDirect3DApp& kDirect3DApp) = delete;
		~InitDirect3DApp();

		virtual bool Initialize() override;

	private:

		virtual void OnResize() override;
		virtual void Update(const GameTimer& kGameTimer) override;
		virtual void Draw(const GameTimer& kGameTimer) override;

		virtual void OnMouseDown(WPARAM btnState, int iMousePositionX, int iMousePositionY) override;
		virtual void OnMouseUp(WPARAM btnState, int iMousePositionX, int iMousePositionY) override;
		virtual void OnMouseMove(WPARAM btnState, int iMousePositionX, int iMousePositionY) override;

		void BuildDescriptorHeaps();
		void BuildConstantBuffers();
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildBoxGeometry();
		void BuildPSO();

	private:

		ComPtr<ID3D12RootSignature> m_pxRootSignature = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_pxCBVHeap = nullptr;
		ComPtr<ID3DBlob> m_pxVertexShaderByteCode = nullptr;
		ComPtr<ID3DBlob> m_pxPixelShaderByteCode = nullptr;
		ComPtr<ID3D12PipelineState> m_pxPipelineState = nullptr;

		std::unique_ptr<UploadBuffer<ObjectConstants>> m_pxObjectConstantBuffer = nullptr;
		std::unique_ptr<MeshGeometry> m_pxBoxGeometry = nullptr;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

		XMFLOAT4X4 m_worldMatrix = MathHelper::Identity4x4();
		XMFLOAT4X4 m_viewMatrix = MathHelper::Identity4x4();
		XMFLOAT4X4 m_projectionMatrix = MathHelper::Identity4x4();

		POINT m_lastMousePosition;

		float m_fTheta = 1.5f * XM_PI;
		float m_fPhi = XM_PIDIV4;
		float m_fRadius = 5.0f;

}; // InitDirect3DApp

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
};

InitDirect3DApp::~InitDirect3DApp()
{
};

bool InitDirect3DApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	};

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* pCommandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(pCommandLists), pCommandLists);

	FlushCommandQueue();

	return true;
};

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX newProjectionMatrix = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projectionMatrix, newProjectionMatrix);
};

void InitDirect3DApp::Update(const GameTimer& kGameTimer)
{
	// Convert Spherical coords to Cartesian coordinates
	float fX = m_fRadius * sin(m_fPhi) * cosf(m_fTheta);
	float fY = m_fRadius * sin(m_fPhi) * sinf(m_fTheta);
	float fZ = m_fRadius * cosf(m_fPhi);

	// Build view matrix
	XMVECTOR position = XMVectorSet(fX, fY, fZ, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);

	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_worldMatrix);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX worldViewProjectionMatrix = worldMatrix * viewMatrix * projectionMatrix;

	ObjectConstants objectConstants;
	XMStoreFloat4x4(&objectConstants.m_worldViewProjection, XMMatrixTranspose(worldViewProjectionMatrix));
	m_pxObjectConstantBuffer->CopyData(0, objectConstants);
};

void InitDirect3DApp::Draw(const GameTimer& kGameTimer)
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), m_pxPipelineState.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_pxCBVHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_pxRootSignature.Get());

	mCommandList->IASetVertexBuffers(0, 1, &m_pxBoxGeometry->VertexBufferView());
	mCommandList->IASetIndexBuffer(&m_pxBoxGeometry->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->SetGraphicsRootDescriptorTable(0, m_pxCBVHeap->GetGPUDescriptorHandleForHeapStart());

	mCommandList->DrawIndexedInstanced(m_pxBoxGeometry->DrawArgs["box"].IndexCount, 1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
};

void InitDirect3DApp::OnMouseDown(WPARAM btnState, int iMousePositionX, int iMousePositionY)
{
	m_lastMousePosition.x = iMousePositionX;
	m_lastMousePosition.y = iMousePositionY;

	SetCapture(mhMainWnd);
};

void InitDirect3DApp::OnMouseUp(WPARAM btnState, int iMousePositionX, int iMousePositionY)
{
	ReleaseCapture();
};

void InitDirect3DApp::OnMouseMove(WPARAM btnState, int iMousePositionX, int iMousePositionY)
{
	if ((btnState&MK_LBUTTON) != 0)
	{
		float fDeltaX = XMConvertToRadians(0.25 * static_cast<float>(iMousePositionX - m_lastMousePosition.x));
		float fDeltaY = XMConvertToRadians(0.25 * static_cast<float>(iMousePositionY - m_lastMousePosition.y));

		m_fTheta += fDeltaX;
		m_fPhi += fDeltaY;

		m_fPhi = MathHelper::Clamp(m_fPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState&MK_RBUTTON) != 0)
	{
		float fDeltaX = 0.005f * static_cast<float>(iMousePositionX - m_lastMousePosition.x);
		float fDeltaY = 0.005f * static_cast<float>(iMousePositionY - m_lastMousePosition.y);

		m_fRadius += fDeltaX - fDeltaY;

		m_fRadius = MathHelper::Clamp(m_fRadius, 3.0f, 15.0f);
	};

	m_lastMousePosition.x = iMousePositionX;
	m_lastMousePosition.y = iMousePositionY;
};

void InitDirect3DApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	ZeroMemory(&cbvHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_pxCBVHeap)));
};

void InitDirect3DApp::BuildConstantBuffers()
{
	m_pxObjectConstantBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT uiObjectConstantBufferByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = m_pxObjectConstantBuffer->Resource()->GetGPUVirtualAddress();
	int iBoxConstantBufferIndex = 0;

	constantBufferAddress += iBoxConstantBufferIndex * uiObjectConstantBufferByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc;
	ZeroMemory(&constantBufferViewDesc, sizeof(D3D12_CONSTANT_BUFFER_VIEW_DESC));

	constantBufferViewDesc.BufferLocation = constantBufferAddress;
	constantBufferViewDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(&constantBufferViewDesc, m_pxCBVHeap->GetCPUDescriptorHandleForHeapStart());
};

void InitDirect3DApp::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE constantBufferViewTable;
	constantBufferViewTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &constantBufferViewTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	};
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&m_pxRootSignature)));
};

void InitDirect3DApp::BuildShadersAndInputLayout()
{
	HRESULT hr;

	m_pxVertexShaderByteCode = d3dUtil::CompileShader(L"Shader\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_pxPixelShaderByteCode = d3dUtil::CompileShader(L"Shader\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
};

void InitDirect3DApp::BuildBoxGeometry()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT kuiVertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT kuiIndexBufferByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_pxBoxGeometry = std::make_unique<MeshGeometry>();
	m_pxBoxGeometry->Name = "Geometry";

	ThrowIfFailed(D3DCreateBlob(kuiIndexBufferByteSize, &m_pxBoxGeometry->VertexBufferCPU));
	CopyMemory(m_pxBoxGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), kuiVertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(kuiIndexBufferByteSize, &m_pxBoxGeometry->IndexBufferCPU));
	CopyMemory(m_pxBoxGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), kuiIndexBufferByteSize);

	m_pxBoxGeometry->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), kuiVertexBufferByteSize, m_pxBoxGeometry->VertexBufferUploader);
	m_pxBoxGeometry->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), kuiIndexBufferByteSize, m_pxBoxGeometry->IndexBufferUploader);

	m_pxBoxGeometry->VertexByteStride = sizeof(Vertex);
	m_pxBoxGeometry->VertexBufferByteSize = kuiVertexBufferByteSize;
	m_pxBoxGeometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_pxBoxGeometry->IndexBufferByteSize = kuiIndexBufferByteSize;

	SubmeshGeometry submesh;
	ZeroMemory(&submesh, sizeof(SubmeshGeometry));
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_pxBoxGeometry->DrawArgs["box"] = submesh;
};

void InitDirect3DApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_pxRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_pxVertexShaderByteCode->GetBufferPointer()),
		m_pxVertexShaderByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_pxPixelShaderByteCode->GetBufferPointer()),
		m_pxPixelShaderByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pxPipelineState)));
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
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	};

};
