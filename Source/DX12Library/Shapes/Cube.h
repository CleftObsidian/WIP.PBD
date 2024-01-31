#pragma once

#include "Shape.h"

namespace DX12Library
{
	class Cube : public Shape
	{
	public:
		Cube(void) = delete;
		Cube(_In_ XMVECTOR& position);
		~Cube() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual VertexPosColor* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

		virtual void PredictPosition(_In_ FLOAT deltaTime);
		virtual void SolveSelfDistanceConstraints(void);
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape);
		virtual void SolveFloorConstraint(void);
		virtual void UpdateVertices(_In_ FLOAT deltaTime);

		XMVECTOR* GetPositionPredictions(void);
		XMVECTOR* GetPositionsBeforeUpdate(void);

	private:
		static constexpr UINT NUM_VERTICES = 8;
		VertexPosColor m_vertices[NUM_VERTICES] =
		{
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
			{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
			{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
			{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
			{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
			{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
			{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
		};

		static constexpr UINT NUM_INDICES = 36;
		static constexpr WORD ms_indicies[NUM_INDICES] =
		{
			0, 1, 2, 0, 2, 3,
			4, 6, 5, 4, 7, 6,
			4, 5, 1, 4, 1, 0,
			3, 2, 6, 3, 6, 7,
			1, 5, 6, 1, 6, 2,
			4, 0, 3, 4, 3, 7
		};

		float m_restLengths[NUM_INDICES];

		XMVECTOR m_p[NUM_VERTICES];		// Predictions for positions of vertices
		XMVECTOR m_x[NUM_VERTICES];		// Positions of vertices before update
		
		XMVECTOR m_velocities[NUM_VERTICES];
	};
}