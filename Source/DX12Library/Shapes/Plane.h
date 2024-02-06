#pragma once

#include "Shape.h"

namespace DX12Library
{
	class Plane : public Shape
	{
	public:
		Plane(void) = delete;
		Plane(_In_ const XMVECTOR& position);
		virtual ~Plane() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void);
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void);

		virtual Vertex* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

		virtual void PredictPosition(_In_ FLOAT deltaTime);
		virtual void SolveSelfDistanceConstraints(void);
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape);
		virtual void SolveFloorConstraint(void);
		virtual void UpdateVertices(_In_ FLOAT deltaTime);

	private:
		static ComPtr<ID3D12Resource> m_vertexBuffer;
		static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		static ComPtr<ID3D12Resource> m_indexBuffer;
		static D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		static constexpr UINT NUM_VERTICES = 4;
		Vertex m_vertices[NUM_VERTICES] =
		{
			{ XMFLOAT3(10.0f, 0.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 0
			{ XMFLOAT3(-10.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 1
			{ XMFLOAT3(-10.0f, 0.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 2
			{ XMFLOAT3(10.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 3
		};

		static constexpr UINT NUM_INDICES = 12;
		static constexpr WORD ms_indices[NUM_INDICES] =
		{
			0, 1, 2, 0, 3, 1,
			0, 2, 1, 0, 1, 3
		};
	};
}