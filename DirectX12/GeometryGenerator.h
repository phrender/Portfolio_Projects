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
			public:

				Vertex()
				{
				};

				Vertex(const DirectX::XMFLOAT3& kPosition, const DirectX::XMFLOAT3& kNormal, const DirectX::XMFLOAT3& kTangentU, const DirectX::XMFLOAT2& kUVCoord) : m_position(kPosition), m_normal(kNormal), m_tangentU(kTangentU), m_textureCoordinate(kUVCoord)
				{
				};

				Vertex(float fPositionX, float fPositionY, float fPositionZ, 
					   float fNormalX, float fNormalY, float fNormalZ, 
					   float fTangentX, float fTangentY, float fTangentZ, 
					   float fU, float fV) : 
					   m_position(fPositionX, fPositionY, fPositionZ),
					   m_normal(fNormalX, fNormalY, fNormalZ),
					   m_tangentU(fTangentX, fTangentY, fTangentZ),
					   m_textureCoordinate(fU, fV)
				{
				};

			public:

				DirectX::XMFLOAT3 m_position;
				DirectX::XMFLOAT3 m_normal;
				DirectX::XMFLOAT3 m_tangentU;
				DirectX::XMFLOAT2 m_textureCoordinate;

			private:

		}; // Vertex

		struct MeshData
		{
			public:

				std::vector<uint16>& GetIndices16()
				{
					if (m_indices16.empty())
					{
						m_indices16.resize(m_indices32.size());

						for (size_t i = 0; i < m_indices32.size(); ++i)
						{
							m_indices16[i] = static_cast<uint16>(m_indices32[i]);
						};
					};

					return m_indices16;
				}

			public:

				std::vector<Vertex> m_vertices;
				std::vector<uint32> m_indices32;

			private:

				std::vector<uint16> m_indices16;

		}; // MeshData

	public:
		
		MeshData CreateBox(float fBoxWidth, float fBoxHeight, float fDepth, uint32 ui32NumSubdivisions);
		MeshData CreateSphere(float fRadius, uint32 ui32SliceCount, uint32 ui32StackCount);
		MeshData CreateGeosphere(float fRadius, uint32 ui32NumSubdivision);
		MeshData CreateCylinder(float fBottomRadius, float fTopRadius, float fHeight, uint32 ui32SliceCount, uint32 ui32StackCount);
		MeshData CreateGrid(float fGridWidth, float fGridDepth, uint32 ui32Rows, uint32 ui32Columns);
		MeshData CreateQuad(float fPositionX, float fPositionY, float fQuadWidth, float fQuadHeight, float fQuadDepth);

	private:

		void Subdivide(MeshData meshData);
		Vertex MidPoint(const Vertex& kVertex0, const Vertex& kVertex1);
		void BuildCylinderTopCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 uiSliceCount, uint32 uiStackCount, MeshData& meshData);
		void BuildCylinderBottomCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 uiSliceCount, uint32 uiStackCount, MeshData& meshData);


}; // GeometryGenerator

#endif // GEOMETRYGENERATOR_H_INCLUDED