#include "Waves.h"
#include <ppl.h>
#include <algorithm>
#include <vector>
#include <cassert>

using namespace DirectX;

Waves::Waves(int iM, int iN, float fDeltaX, float fDeltaT, float fSpeed, float fDamping) :
			m_iNumRows(0), m_iNumColumns(0), 
			m_iVertexCount(0), m_iTriangleCount(0),
			m_fK1(0.0f), m_fK2(0.0f), m_fK3(0.0f),
			m_fTimeStep(0.0f), m_fSpatialStep(0.0f)
{
	m_iNumRows = iM;
	m_iNumColumns = iN;

	m_iVertexCount = iM * iN;
	m_iTriangleCount = (iM - 1) * (iN - 1) * 2;

	m_fTimeStep = fDeltaT;
	m_fSpatialStep = fDeltaX;

	float d = fDamping * fDeltaT + 2.0f;
	float e = (fSpeed * fSpeed) * (fDeltaT * fDeltaT) / (fDeltaX * fDeltaX);
	m_fK1 = (fDamping * fDeltaT - 2.0f) / d;
	m_fK2 = (4.0f - 8.0f * e) / d;
	m_fK3 = (2.0f*e) / d;

	m_previousSolution.resize(iM * iN);
	m_currentSolution.resize(iM * iN);
	m_normals.resize(iM * iN);
	m_tangentX.resize(iM * iN);

	// Generate grid vertices in system memory.

	float halfWidth = (iN - 1) * fDeltaX * 0.5f;
	float halfDepth = (iM - 1) * fDeltaX * 0.5f;
	for (int i = 0; i < iM; ++i)
	{
		float z = halfDepth - i * fDeltaX;
		for (int j = 0; j < iN; ++j)
		{
			float x = -halfWidth + j * fDeltaX;

			m_previousSolution[i * iN + j] = XMFLOAT3(x, 0.0f, z);
			m_currentSolution[i * iN + j] = XMFLOAT3(x, 0.0f, z);
			m_normals[i * iN + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_tangentX[i * iN + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
		};
	};
};

Waves::~Waves()
{
};

int Waves::RowCount()const
{
	return m_iNumRows;
};

int Waves::ColumnCount()const
{
	return m_iNumColumns;
};

int Waves::VertexCount()const
{
	return m_iVertexCount;
};

int Waves::TriangleCount()const
{
	return m_iTriangleCount;
};

float Waves::Width()const
{
	return m_iNumColumns * m_fSpatialStep;
};

float Waves::Depth()const
{
	return m_iNumRows * m_fSpatialStep;
};

void Waves::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= m_fTimeStep)
	{
		// Only update interior points; we use zero boundary conditions.
		concurrency::parallel_for(1, m_iNumRows - 1, [this](int i)
			//for(int i = 1; i < mNumRows-1; ++i)
		{
			for (int j = 1; j < m_iNumColumns - 1; ++j)
			{
				// After this update we will be discarding the old previous
				// buffer, so overwrite that buffer with the new update.
				// Note how we can do this inplace (read/write to same element) 
				// because we won't need prev_ij again and the assignment happens last.

				// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				// Moreover, our +z axis goes "down"; this is just to 
				// keep consistent with our row indices going down.

				m_previousSolution[i * m_iNumColumns + j].y =
					m_fK1*m_previousSolution[i * m_iNumColumns + j].y +
					m_fK2*m_currentSolution[i * m_iNumColumns + j].y +
					m_fK3*(m_currentSolution[(i + 1) * m_iNumColumns + j].y +
						m_currentSolution[(i - 1) * m_iNumColumns + j].y +
						m_currentSolution[i * m_iNumColumns + j + 1].y +
						m_currentSolution[i * m_iNumColumns + j - 1].y);
			}
		});

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(m_previousSolution, m_currentSolution);

		t = 0.0f; // reset time

				  //
				  // Compute normals using finite difference scheme.
				  //
		concurrency::parallel_for(1, m_iNumRows - 1, [this](int i)
			//for(int i = 1; i < mNumRows - 1; ++i)
		{
			for (int j = 1; j < m_iNumColumns - 1; ++j)
			{
				float l = m_currentSolution[i * m_iNumColumns + j - 1].y;
				float r = m_currentSolution[i * m_iNumColumns + j + 1].y;
				float t = m_currentSolution[(i - 1) * m_iNumColumns + j].y;
				float b = m_currentSolution[(i + 1) * m_iNumColumns + j].y;
				m_normals[i * m_iNumColumns + j].x = -r + l;
				m_normals[i * m_iNumColumns + j].y = 2.0f * m_fSpatialStep;
				m_normals[i * m_iNumColumns + j].z = b - t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_normals[i * m_iNumColumns + j]));
				XMStoreFloat3(&m_normals[i * m_iNumColumns + j], n);

				m_tangentX[i * m_iNumColumns + j] = XMFLOAT3(2.0f * m_fSpatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_tangentX[i * m_iNumColumns + j]));
				XMStoreFloat3(&m_tangentX[i * m_iNumColumns + j], T);
			};
		});
	};
};

void Waves::Disturb(int i, int j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < m_iNumRows - 2);
	assert(j > 1 && j < m_iNumColumns - 2);

	float halfMag = 0.5f*magnitude;

	// Disturb the ijth vertex height and its neighbors.
	m_currentSolution[i * m_iNumColumns + j].y += magnitude;
	m_currentSolution[i * m_iNumColumns + j + 1].y += halfMag;
	m_currentSolution[i * m_iNumColumns + j - 1].y += halfMag;
	m_currentSolution[(i + 1) * m_iNumColumns + j].y += halfMag;
	m_currentSolution[(i - 1) * m_iNumColumns + j].y += halfMag;
}
