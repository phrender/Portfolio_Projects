#ifndef GEOMETRYGENERATOR_H_INCLUDED
#define GEOMETRYGENERATOR_H_INCLUDED

#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>

class GeometryGenerator
{
	public:
	
	    using uint16 = std::uint16_t;
	    using uint32 = std::uint32_t;
	
		struct Vertex
		{
			Vertex(){}
	        Vertex( const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n,  const DirectX::XMFLOAT3& t,  const DirectX::XMFLOAT2& uv) : m_position(p), m_normal(n),  m_tangentU(t), m_texC(uv)
			{
			};

			Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v) : m_position(px, py, pz), m_normal(nx, ny, nz), m_tangentU(tx, ty, tz),  m_texC(u, v)
			{
			};
	
	        DirectX::XMFLOAT3 m_position;
	        DirectX::XMFLOAT3 m_normal;
	        DirectX::XMFLOAT3 m_tangentU;
	        DirectX::XMFLOAT2 m_texC;
		};
	
		struct MeshData
		{
			std::vector<Vertex> m_vertices;
	        std::vector<uint32> m_indices32;
	
	        std::vector<uint16>& GetIndices16()
	        {
				if(m_indices16.empty())
				{
					m_indices16.resize(m_indices32.size());
					for(size_t i = 0; i < m_indices32.size(); ++i)
						m_indices16[i] = static_cast<uint16>(m_indices32[i]);
				};
	
				return m_indices16;
			};
	
		private:
			std::vector<uint16> m_indices16;
		};
	

	    MeshData CreateBox(float fWidth, float fHeight, float fDepth, uint32 uiNumSubdivisions);
	    MeshData CreateSphere(float fRadius, uint32 uiSliceCount, uint32 uiStackCount);
	    MeshData CreateGeosphere(float fRadius, uint32 uiNumSubdivisions);
	    MeshData CreateCylinder(float fBottomRadius, float fTopRadius, float fHeight, uint32 uiSliceCount, uint32 uiStackCount);
	    MeshData CreateGrid(float fGridWidth, float fDepth, uint32 m, uint32 n);
	    MeshData CreateQuad(float fPositionX, float fPositionY, float fQuadWidth, float fQuadHeight, float fDepth);
	
	private:
		void Subdivide(MeshData& meshData);
	    Vertex MidPoint(const Vertex& vertex0, const Vertex& vertex1);
	    void BuildCylinderTopCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 ui32SliceCount, uint32 ui32StackCount, MeshData& meshData);
	    void BuildCylinderBottomCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 ui32SliceCount, uint32 ui32StackCount, MeshData& meshData);
};

#endif // GEOMETRYGENERATOR_H_INCLUDED