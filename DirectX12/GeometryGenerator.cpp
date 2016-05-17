#include "GeometryGenerator.h"
#include <algorithm>

using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float fBoxWidth, float fBoxHeight, float fDepth, uint32 ui32NumSubdivisions)
{
	MeshData meshData;

	//
	// Create the vertices.
	//

	Vertex vertex[24];

	float w2 = 0.5f * fBoxWidth;
	float h2 = 0.5f * fBoxHeight;
	float d2 = 0.5f * fDepth;

	// Fill in the front face vertex data.
	vertex[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vertex[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vertex[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	vertex[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	vertex[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	vertex[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vertex[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vertex[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	vertex[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vertex[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vertex[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	vertex[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	vertex[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	vertex[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vertex[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vertex[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	vertex[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vertex[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vertex[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vertex[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	vertex[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	vertex[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	vertex[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	vertex[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.m_vertices.assign(&vertex[0], &vertex[24]);

	//
	// Create the indices.
	//

	uint32 index[36];

	// Fill in the front face index data
	index[0] = 0; index[1] = 1; index[2] = 2;
	index[3] = 0; index[4] = 2; index[5] = 3;

	// Fill in the back face index data
	index[6] = 4; index[7] = 5; index[8] = 6;
	index[9] = 4; index[10] = 6; index[11] = 7;

	// Fill in the top face index data
	index[12] = 8; index[13] = 9; index[14] = 10;
	index[15] = 8; index[16] = 10; index[17] = 11;

	// Fill in the bottom face index data
	index[18] = 12; index[19] = 13; index[20] = 14;
	index[21] = 12; index[22] = 14; index[23] = 15;

	// Fill in the left face index data
	index[24] = 16; index[25] = 17; index[26] = 18;
	index[27] = 16; index[28] = 18; index[29] = 19;

	// Fill in the right face index data
	index[30] = 20; index[31] = 21; index[32] = 22;
	index[33] = 20; index[34] = 22; index[35] = 23;

	meshData.m_indices32.assign(&index[0], &index[36]);

	// Put a cap on the number of subdivisions.
	ui32NumSubdivisions = std::min<uint32>(ui32NumSubdivisions, 6u);

	for (uint32 i = 0; i < ui32NumSubdivisions; ++i)
		Subdivide(meshData);

	return meshData;
};

GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float fRadius, uint32 ui32SliceCount, uint32 ui32StackCount)
{
	MeshData meshData;

	Vertex topVertex(0.0f, +fRadius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -fRadius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.m_vertices.push_back(topVertex);

	float phiStep = XM_PI / ui32StackCount;
	float thetaStep = 2.0f * XM_PI / ui32SliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= ui32StackCount - 1; ++i)
	{
		float phi = i*phiStep;

		// Vertices of ring.
		for (uint32 j = 0; j <= ui32SliceCount; ++j)
		{
			float theta = j * thetaStep;

			Vertex v;

			// spherical to cartesian
			v.m_position.x = fRadius * sinf(phi)*cosf(theta);
			v.m_position.y = fRadius * cosf(phi);
			v.m_position.z = fRadius * sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.m_tangentU.x = -fRadius * sinf(phi) * sinf(theta);
			v.m_tangentU.y = 0.0f;
			v.m_tangentU.z = +fRadius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.m_tangentU);
			XMStoreFloat3(&v.m_tangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.m_position);
			XMStoreFloat3(&v.m_normal, XMVector3Normalize(p));

			v.m_textureCoordinate.x = theta / XM_2PI;
			v.m_textureCoordinate.y = phi / XM_PI;

			meshData.m_vertices.push_back(v);
		}
	}

	meshData.m_vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32 i = 1; i <= ui32SliceCount; ++i)
	{
		meshData.m_indices32.push_back(0);
		meshData.m_indices32.push_back(i + 1);
		meshData.m_indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32 baseIndex = 1;
	uint32 ringVertexCount = ui32SliceCount + 1;
	for (uint32 i = 0; i < ui32StackCount - 2; ++i)
	{
		for (uint32 j = 0; j < ui32SliceCount; ++j)
		{
			meshData.m_indices32.push_back(baseIndex + i * ringVertexCount + j);
			meshData.m_indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.m_indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.m_indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.m_indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.m_indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	uint32 southPoleIndex = (uint32)meshData.m_vertices.size() - 1;

	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < ui32SliceCount; ++i)
	{
		meshData.m_indices32.push_back(southPoleIndex);
		meshData.m_indices32.push_back(baseIndex + i);
		meshData.m_indices32.push_back(baseIndex + i + 1);
	}

	return meshData;
};

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float fRadius, uint32 ui32NumSubdivision)
{
	MeshData meshData;

	// Put a cap on the number of subdivisions.
	ui32NumSubdivision = std::min<uint32>(ui32NumSubdivision, 6u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

	uint32 k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.m_vertices.resize(12);
	meshData.m_indices32.assign(&k[0], &k[60]);

	for (uint32 i = 0; i < 12; ++i)
	{
		meshData.m_vertices[i].m_position = pos[i];
	};

	for (uint32 i = 0; i < ui32NumSubdivision; ++i)
	{
		Subdivide(meshData);
	};

	// Project vertices onto sphere and scale.
	for (uint32 i = 0; i < meshData.m_vertices.size(); ++i)
	{
		// Project onto unit sphere.
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.m_vertices[i].m_position));

		// Project onto sphere.
		XMVECTOR p = fRadius * n;

		XMStoreFloat3(&meshData.m_vertices[i].m_position, p);
		XMStoreFloat3(&meshData.m_vertices[i].m_normal, n);

		float theta = atan2f(meshData.m_vertices[i].m_position.z, meshData.m_vertices[i].m_position.x);

		// Put in [0, 2pi].
		if (theta < 0.0f)
			theta += XM_2PI;

		float phi = acosf(meshData.m_vertices[i].m_position.y / fRadius);

		meshData.m_vertices[i].m_textureCoordinate.x = theta / XM_2PI;
		meshData.m_vertices[i].m_textureCoordinate.y = phi / XM_PI;

		// Partial derivative of P with respect to theta
		meshData.m_vertices[i].m_tangentU.x = -fRadius * sinf(phi) * sinf(theta);
		meshData.m_vertices[i].m_tangentU.y = 0.0f;
		meshData.m_vertices[i].m_tangentU.z = +fRadius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.m_vertices[i].m_tangentU);
		XMStoreFloat3(&meshData.m_vertices[i].m_tangentU, XMVector3Normalize(T));
	};

	return meshData;
};

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float fBottomRadius, float fTopRadius, float fHeight, uint32 ui32SliceCount, uint32 ui32StackCount)
{
	MeshData meshData;

	float fStackHeight = fHeight / ui32StackCount;

	float fRadiusStep = (fTopRadius - fBottomRadius) / ui32StackCount;

	uint32 ringCount = ui32StackCount + 1;

	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * fHeight + i * fStackHeight;
		float r = fBottomRadius + i * fRadiusStep;

		// vertices of ring
		float dTheta = 2.0f * XM_PI / ui32SliceCount;
		for (uint32 j = 0; j <= ui32SliceCount; ++j)
		{
			Vertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.m_position = XMFLOAT3(r * c, y, r * s);

			vertex.m_textureCoordinate.x = (float)j / ui32SliceCount;
			vertex.m_textureCoordinate.y = 1.0f - (float)i / ui32StackCount;

			vertex.m_tangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = fBottomRadius - fTopRadius;
			XMFLOAT3 bitangent(dr * c, -fHeight, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.m_tangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.m_normal, N);

			meshData.m_vertices.push_back(vertex);
		}
	}

	uint32 ringVertexCount = ui32SliceCount + 1;

	for (uint32 i = 0; i < ui32StackCount; ++i)
	{
		for (uint32 j = 0; j < ui32SliceCount; ++j)
		{
			meshData.m_indices32.push_back(i * ringVertexCount + j);
			meshData.m_indices32.push_back((i + 1) * ringVertexCount + j);
			meshData.m_indices32.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.m_indices32.push_back(i * ringVertexCount + j);
			meshData.m_indices32.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.m_indices32.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(fBottomRadius, fTopRadius, fHeight, ui32SliceCount, ui32StackCount, meshData);
	BuildCylinderBottomCap(fBottomRadius, fTopRadius, fHeight, ui32SliceCount, ui32StackCount, meshData);

	return meshData;
};

GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float fGridWidth, float fGridDepth, uint32 ui32Rows, uint32 ui32Columns)
{
	MeshData meshData;

	uint32 vertexCount = ui32Rows * ui32Columns;
	uint32 faceCount = (ui32Rows - 1) * (ui32Columns - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f*fGridWidth;
	float halfDepth = 0.5f*fGridDepth;

	float dx = fGridWidth / (ui32Columns - 1);
	float dz = fGridDepth / (ui32Rows - 1);

	float du = 1.0f / (ui32Columns - 1);
	float dv = 1.0f / (ui32Rows - 1);

	meshData.m_vertices.resize(vertexCount);
	for (uint32 i = 0; i < ui32Rows; ++i)
	{
		float z = halfDepth - i*dz;
		for (uint32 j = 0; j < ui32Columns; ++j)
		{
			float x = -halfWidth + j*dx;

			meshData.m_vertices[i * ui32Columns + j].m_position = XMFLOAT3(x, 0.0f, z);
			meshData.m_vertices[i * ui32Columns + j].m_normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.m_vertices[i * ui32Columns + j].m_tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			meshData.m_vertices[i * ui32Columns + j].m_textureCoordinate.x = j * du;
			meshData.m_vertices[i * ui32Columns + j].m_textureCoordinate.y = i * dv;
		}
	}

	meshData.m_indices32.resize(faceCount * 3);

	uint32 k = 0;
	for (uint32 i = 0; i < ui32Rows - 1; ++i)
	{
		for (uint32 j = 0; j < ui32Columns - 1; ++j)
		{
			meshData.m_indices32[k] = i * ui32Columns + j;
			meshData.m_indices32[k + 1] = i * ui32Columns + j + 1;
			meshData.m_indices32[k + 2] = (i + 1) * ui32Columns + j;

			meshData.m_indices32[k + 3] = (i + 1) * ui32Columns + j;
			meshData.m_indices32[k + 4] = i * ui32Columns + j + 1;
			meshData.m_indices32[k + 5] = (i + 1) * ui32Columns + j + 1;

			k += 6; // next quad
		};
	};

	return meshData;
};

GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float fPositionX, float fPositionY, float fQuadWidth, float fQuadHeight, float fQuadDepth)
{
	MeshData meshData;

	meshData.m_vertices.resize(4);
	meshData.m_indices32.resize(6);

	// Position coordinates specified in NDC space.
	meshData.m_vertices[0] = Vertex(
		fPositionX, fPositionY - fQuadHeight, fQuadDepth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.m_vertices[1] = Vertex(
		fPositionX, fPositionY, fQuadDepth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.m_vertices[2] = Vertex(
		fPositionX + fQuadWidth, fPositionY, fQuadDepth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.m_vertices[3] = Vertex(
		fPositionX + fQuadWidth, fPositionY - fQuadHeight, fQuadDepth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.m_indices32[0] = 0;
	meshData.m_indices32[1] = 1;
	meshData.m_indices32[2] = 2;

	meshData.m_indices32[3] = 0;
	meshData.m_indices32[4] = 2;
	meshData.m_indices32[5] = 3;

	return meshData;
};

void GeometryGenerator::Subdivide(MeshData meshData)
{
	MeshData inputCopy = meshData;

	meshData.m_vertices.resize(0);
	meshData.m_indices32.resize(0);
	
	uint32 numTris = (uint32)inputCopy.m_indices32.size() / 3;

	for (uint32 i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.m_vertices[inputCopy.m_indices32[i * 3 + 0]];
		Vertex v1 = inputCopy.m_vertices[inputCopy.m_indices32[i * 3 + 1]];
		Vertex v2 = inputCopy.m_vertices[inputCopy.m_indices32[i * 3 + 2]];

		Vertex m0 = MidPoint(v0, v1);
		Vertex m1 = MidPoint(v1, v2);
		Vertex m2 = MidPoint(v0, v2);

		meshData.m_vertices.push_back(v0); // 0
		meshData.m_vertices.push_back(v1); // 1
		meshData.m_vertices.push_back(v2); // 2
		meshData.m_vertices.push_back(m0); // 3
		meshData.m_vertices.push_back(m1); // 4
		meshData.m_vertices.push_back(m2); // 5

		meshData.m_indices32.push_back(i * 6 + 0);
		meshData.m_indices32.push_back(i * 6 + 3);
		meshData.m_indices32.push_back(i * 6 + 5);

		meshData.m_indices32.push_back(i * 6 + 3);
		meshData.m_indices32.push_back(i * 6 + 4);
		meshData.m_indices32.push_back(i * 6 + 5);

		meshData.m_indices32.push_back(i * 6 + 5);
		meshData.m_indices32.push_back(i * 6 + 4);
		meshData.m_indices32.push_back(i * 6 + 2);

		meshData.m_indices32.push_back(i * 6 + 3);
		meshData.m_indices32.push_back(i * 6 + 1);
		meshData.m_indices32.push_back(i * 6 + 4);
	};
};

GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& kVertex0, const Vertex& kVertex1)
{
	XMVECTOR p0 = XMLoadFloat3(&kVertex0.m_position);
	XMVECTOR p1 = XMLoadFloat3(&kVertex1.m_position);

	XMVECTOR n0 = XMLoadFloat3(&kVertex0.m_normal);
	XMVECTOR n1 = XMLoadFloat3(&kVertex1.m_normal);

	XMVECTOR tan0 = XMLoadFloat3(&kVertex0.m_tangentU);
	XMVECTOR tan1 = XMLoadFloat3(&kVertex1.m_tangentU);

	XMVECTOR tex0 = XMLoadFloat2(&kVertex0.m_textureCoordinate);
	XMVECTOR tex1 = XMLoadFloat2(&kVertex1.m_textureCoordinate);

	XMVECTOR pos = 0.5f*(p0 + p1);
	XMVECTOR normal = XMVector3Normalize(0.5f*(n0 + n1));
	XMVECTOR tangent = XMVector3Normalize(0.5f*(tan0 + tan1));
	XMVECTOR tex = 0.5f*(tex0 + tex1);

	Vertex vertex;
	XMStoreFloat3(&vertex.m_position, pos);
	XMStoreFloat3(&vertex.m_normal, normal);
	XMStoreFloat3(&vertex.m_tangentU, tangent);
	XMStoreFloat2(&vertex.m_textureCoordinate, tex);

	return vertex;
};

void GeometryGenerator::BuildCylinderTopCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 uiSliceCount, uint32 uiStackCount, MeshData& meshData)
{
	uint32 ui32BaseIndex = (uint32)meshData.m_vertices.size();

	float y = 0.5f * fHeight;
	float dTheta = 2.0f * XM_PI / uiSliceCount;

	for (uint32 i = 0; i <= uiSliceCount; ++i)
	{
		float x = fTopRadius * cosf(i * dTheta);
		float z = fTopRadius * sinf(i * dTheta);

		float u = x / fHeight + 0.5f;
		float v = z / fHeight + 0.5f;

		meshData.m_vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	};

	meshData.m_vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	uint32 ui32CenterIndex = (uint32)meshData.m_vertices.size() - 1;

	for (uint32 i = 0; i < uiSliceCount; ++i)
	{
		meshData.m_indices32.push_back(ui32CenterIndex);
		meshData.m_indices32.push_back(ui32BaseIndex + i + 1);
		meshData.m_indices32.push_back(ui32BaseIndex + i);
	};
};

void GeometryGenerator::BuildCylinderBottomCap(float fBottomRadius, float fTopRadius, float fHeight, uint32 uiSliceCount, uint32 uiStackCount, MeshData& meshData)
{
	uint32 ui32BaseIndex = (uint32)meshData.m_vertices.size();
	float y = -0.5f * fHeight;
	float dTheta = 2.0f * XM_PI / uiSliceCount;

	for (uint32 i = 0; i <= uiSliceCount; ++i)
	{
		float x = fBottomRadius * cosf(i*dTheta);
		float z = fBottomRadius * sinf(i*dTheta);

		float u = x / fHeight + 0.5f;
		float v = z / fHeight + 0.5f;

		meshData.m_vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	meshData.m_vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	uint32 ui32CenterIndex = (uint32)meshData.m_vertices.size() - 1;

	for (uint32 i = 0; i < uiSliceCount; ++i)
	{
		meshData.m_indices32.push_back(ui32CenterIndex);
		meshData.m_indices32.push_back(ui32BaseIndex + i);
		meshData.m_indices32.push_back(ui32BaseIndex + i + 1);
	}
};
