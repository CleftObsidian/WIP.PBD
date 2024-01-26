#include "Cube.h"

namespace DX12Library
{
	Cube::Cube(_In_ XMVECTOR& position)
		: Shape(position)
		, velocity(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
	{
	}

	void Cube::Initialize(_In_ ID3D12Device* pDevice)
	{
		{
			const UINT vertexBufferSize = sizeof(ms_vertices);

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)));

			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, ms_vertices, vertexBufferSize);
			m_vertexBuffer->Unmap(0, nullptr);

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		{
			const UINT indexBufferSize = sizeof(ms_indicies);

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_indexBuffer)));

			UINT8* pIndexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
			memcpy(pIndexDataBegin, ms_indicies, indexBufferSize);
			m_indexBuffer->Unmap(0, nullptr);

			m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = indexBufferSize;
		}
	}

	void Cube::Update(_In_ FLOAT deltaTime)
	{
		// Get world position vector(x) from world transform matrix
		XMVECTOR s;	// not use
		XMVECTOR r;	// not use
		XMVECTOR x;
		XMMatrixDecompose(&s, &r, &x, m_world);

		// Estimate next position (p) only with gravity
		velocity = XMVectorSetY(velocity, XMVectorGetY(velocity) + deltaTime * -9.81f);		// v <- v + dt * (gravity acceleration)
		XMVECTOR p = x + (deltaTime * velocity);											// p <- x + dt * v

		// Solve constraint (now just floor plane)
		constexpr size_t SOLVER_ITERATION = 100;
		for (size_t i = 0; i < SOLVER_ITERATION; ++i)
		{
			if (XMVectorGetY(p) < 0.0f)
			{
				// C(p) = p_y
				XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
				float lambda = -XMVectorGetY(p);
				XMVECTOR dp = lambda * gradC;
				p += dp;
			}
		}

		// Update velocity and position
		velocity = (p - x) / deltaTime;
		XMVECTOR dx = p - x;
		
		m_world *= XMMatrixTranslationFromVector(dx);
	}

	UINT Cube::GetNumVertices() const
	{
		return NUM_VERTICES;
	}

	UINT Cube::GetNumIndices() const
	{
		return NUM_INDICES;
	}
}