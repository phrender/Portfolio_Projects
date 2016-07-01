#ifndef D3DUTIL_H_INCLUDED
#define D3DUTIL_H_INCLUDED
#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

/*
#if defined(_DEBUG)
    #ifndef Assert
    #define Assert(x, description)                                  \
    {                                                               \
        static bool ignoreAssert = false;                           \
        if(!ignoreAssert && !(x))                                   \
        {                                                           \
            Debug::AssertResult result = Debug::ShowAssertDialog(   \
            (L#x), description, AnsiToWString(__FILE__), __LINE__); \
        if(result == Debug::AssertIgnore)                           \
        {                                                           \
            ignoreAssert = true;                                    \
        }                                                           \
                    else if(result == Debug::AssertBreak)           \
        {                                                           \
            __debugbreak();                                         \
        }                                                           \
        }                                                           \
    }
    #endif
#else
    #ifndef Assert
    #define Assert(x, description) 
    #endif
#endif 		
    */

class d3dUtil
{
	public:
	
	    static bool IsKeyDown(int iVirtualKeyCode);
	
	    static std::string ToString(HRESULT hr);
	
	    static UINT CalcConstantBufferByteSize(UINT uiByteSize)
	    {
	        // Constant buffers must be a multiple of the minimum hardware
	        // allocation size (usually 256 bytes).  So round up to nearest
	        // multiple of 256.  We do this by adding 255 and then masking off
	        // the lower 2 bytes which store all bits < 256.
	        // Example: Suppose byteSize = 300.
	        // (300 + 255) & ~255
	        // 555 & ~255
	        // 0x022B & ~0x00ff
	        // 0x022B & 0xff00
	        // 0x0200
	        // 512
	        return (uiByteSize + 255) & ~255;
	    }
	
	    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& kwstrFilename);
	    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pGraphicsCommandList, const void* kInitData, UINT64 ui64ByteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& pUploadBuffer);
		static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& kwstrFilename, const D3D_SHADER_MACRO* kDefines, const std::string& kstrEntrypoint, const std::string& kstrTarget);
};

class DxException
{
	public:
	    DxException() = default;
	    DxException(HRESULT hr, const std::wstring& kwstrFunctionName, const std::wstring& kwstrFilename, int iLineNumber);
	
	    std::wstring ToString() const;
	
	    HRESULT m_hrErrorCode = S_OK;
	    std::wstring m_wstrFunctionName;
	    std::wstring m_wstrFilename;
	    int m_iLineNumber = -1;
};

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
	UINT m_uiIndexCount = 0;
	UINT m_uiStartIndexLocation = 0;
	INT m_iBaseVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh. 
    // This is used in later chapters of the book.
	DirectX::BoundingBox m_bounds;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string m_strName;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> m_pxVertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pxIndexBufferCPU  = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxVertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxIndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxVertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxIndexBufferUploader = nullptr;

    // Data about the buffers.
	UINT m_uiVertexByteStride = 0;
	UINT m_uiVertexBufferByteSize = 0;
	DXGI_FORMAT m_indexFormat = DXGI_FORMAT_R16_UINT;
	UINT m_uiIndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> m_drawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		ZeroMemory(&vbv, sizeof(D3D12_VERTEX_BUFFER_VIEW));
		vbv.BufferLocation = m_pxVertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = m_uiVertexByteStride;
		vbv.SizeInBytes = m_uiVertexBufferByteSize;

		return vbv;
	};

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ZeroMemory(&ibv, sizeof(D3D12_INDEX_BUFFER_VIEW));
		ibv.BufferLocation = m_pxIndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = m_indexFormat;
		ibv.SizeInBytes = m_uiIndexBufferByteSize;

		return ibv;
	};

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		m_pxVertexBufferUploader = nullptr;
		m_pxIndexBufferUploader = nullptr;
	};
};

struct Light
{
    DirectX::XMFLOAT3 m_strength = { 0.5f, 0.5f, 0.5f };
    float m_fFalloffStart = 1.0f;								// point/spot light only
    DirectX::XMFLOAT3 m_direction = { 0.0f, -1.0f, 0.0f };		// directional/spot light only
    float m_fFalloffEnd = 10.0f;								// point/spot light only
    DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };		// point/spot light only
    float m_fSpotPower = 64.0f;									// spot light only
};

#define MaxLights 16

struct MaterialConstants
{
	DirectX::XMFLOAT4 m_diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float m_fRoughness = 0.25f;

	// Used in texture mapping.
	DirectX::XMFLOAT4X4 m_materialTransform = MathHelper::Identity4x4();
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
	// Unique material name for lookup.
	std::string m_strName;

	// Index into constant buffer corresponding to this material.
	int m_iMaterialConstantBufferIndex = -1;

	// Index into SRV heap for diffuse texture.
	int m_iDiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int m_iNormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int m_iNumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading.
	DirectX::XMFLOAT4 m_diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float m_fRoughness = .25f;
	DirectX::XMFLOAT4X4 m_materialTransform = MathHelper::Identity4x4();
};

struct Texture
{
	// Unique material name for lookup.
	std::string m_strName;

	std::wstring m_wstrFilename;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pxUploadHeap = nullptr;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

#endif // D3DUTIL_H_INCLUDED