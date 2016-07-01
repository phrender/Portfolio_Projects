#ifndef WAVES_H_INCLUDED
#define WAVES_H_INCLUDED
#pragma once

#include <vector>
#include <DirectXMath.h>

class Waves
{
	public:
		Waves(int iM, int iN, float fDeltaX, float fDeltaT, float fSpeed, float fDamping);
		Waves(const Waves& kWaves) = delete;
		Waves& operator=(const Waves& kWaves) = delete;
		~Waves();
	
		int RowCount() const;
		int ColumnCount() const;
		int VertexCount() const;
		int TriangleCount() const;
		float Width() const;
		float Depth() const;

		const DirectX::XMFLOAT3& Position(int i)const { return m_currentSolution[i]; }

		// Returns the solution normal at the ith grid point.
		const DirectX::XMFLOAT3& Normal(int i)const { return m_normals[i]; }

		// Returns the unit tangent vector at the ith grid point in the local x-axis direction.
		const DirectX::XMFLOAT3& TangentX(int i)const { return m_tangentX[i]; }

		void Update(float dt);
		void Disturb(int i, int j, float magnitude);

	private:

		int m_iNumRows;
		int m_iNumColumns;

		int m_iVertexCount;
		int m_iTriangleCount;

		// Simulation constants we can precompute.
		float m_fK1;
		float m_fK2;
		float m_fK3;

		float m_fTimeStep;
		float m_fSpatialStep;

		std::vector<DirectX::XMFLOAT3> m_previousSolution;
		std::vector<DirectX::XMFLOAT3> m_currentSolution;
		std::vector<DirectX::XMFLOAT3> m_normals;
		std::vector<DirectX::XMFLOAT3> m_tangentX;

}; // !Waves

#endif // WAVES_H_INCLUDED