#pragma once

#include "Shape.h"

namespace DX12Library
{
	class Plane : public Shape 
	{
	public:
		Plane(void) = delete;
		Plane(_In_ XMVECTOR& position);
		~Plane() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

	private:
		static constexpr UINT NUM_VERTICES = 4;
		static constexpr VertexPosColor ms_vertices[NUM_VERTICES] =
		{
			{ XMFLOAT3(10.0f, 0.0f, 10.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 0
			{ XMFLOAT3(-10.0f, 0.0f, -10.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 1
			{ XMFLOAT3(-10.0f, 0.0f, 10.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 2
			{ XMFLOAT3(10.0f, 0.0f, -10.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) }, // 3
		};

		static constexpr UINT NUM_INDICES = 12;
		static constexpr WORD ms_indicies[NUM_INDICES] =
		{
			0, 1, 2, 0, 3, 1,
			0, 2, 1, 0, 1, 3
		};
	};
}