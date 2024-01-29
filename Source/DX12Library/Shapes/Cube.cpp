#include "Cube.h"

namespace DX12Library
{
	Cube::Cube(_In_ XMVECTOR& position)
		: Shape()
		, m_restLengths()
		, m_velocities{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) }
	{
		XMVECTOR pos[NUM_VERTICES];

		// Initialize vertices position
		for (size_t i = 0; i < NUM_VERTICES; ++i)
		{
			pos[i] = XMLoadFloat3(&m_vertices[i].position);
			pos[i] += position;
			XMStoreFloat3(&m_vertices[i].position, pos[i]);
		}

		// Initialize edges length
		for (size_t i = 1; i < NUM_INDICES; ++i)
		{
			m_restLengths[i] = XMVectorGetX(XMVector3Length(pos[ms_indicies[i - 1]] - pos[ms_indicies[i]]));
		}
	}

	void Cube::Initialize(_In_ ID3D12Device* pDevice)
	{
		{
			const UINT vertexBufferSize = sizeof(m_vertices);

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
			memcpy(pVertexDataBegin, m_vertices, vertexBufferSize);
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
		XMVECTOR x[NUM_VERTICES];
		XMVECTOR p[NUM_VERTICES];

		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			// Get position vector(x)
			x[v] = XMLoadFloat3(&m_vertices[v].position);

			// Estimate next position(p) only considering gravity (Euler Method)
			m_velocities[v] += deltaTime * GRAVITY;			// v <- v + dt * (gravity acceleration)
			p[v] = x[v] + (deltaTime * m_velocities[v]);	// p <- x + dt * v
		}

#ifdef _DEBUG
		LARGE_INTEGER sConstraints;
		QueryPerformanceCounter(&sConstraints);
#endif
		// Solve constraints
		{
			// Iterate solving constraints
			constexpr size_t SOLVER_ITERATION = 10;

			for (size_t count = 0; count < SOLVER_ITERATION; ++count)
			{
				// Solve distance constraints between vertices
				for (size_t idx = 1; idx < NUM_INDICES; ++idx)
				{
					// C(p1, p2) = |p1 - p2| - (rest length) = 0
					XMVECTOR edge = p[ms_indicies[idx - 1]] - p[ms_indicies[idx]];		// p1 - p2
					float distance = XMVectorGetX(XMVector3Length(edge));				// |p1 - p2|
					XMVECTOR normal = XMVector3Normalize(edge);							// (p1 - p2) / |p1 - p2|

					p[ms_indicies[idx - 1]] -= 0.5f * (distance - m_restLengths[idx]) * normal;
					p[ms_indicies[idx]] += 0.5f * (distance - m_restLengths[idx]) * normal;
				}

				// for all vertices
				for (size_t v = 0; v < NUM_VERTICES; ++v)
				{
					// Solve floor(limited y-height) constraint
					if (XMVectorGetY(p[v]) < 0.0f)
					{
						// C(p) = p_y >= 0
						XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
						float lambda = -XMVectorGetY(p[v]);	// lambda = -C(p) / |gradC|^2
						XMVECTOR dp = lambda * gradC;

						p[v] += dp;
					}
				}
			}
		}
#ifdef _DEBUG
		LARGE_INTEGER eConstraints;
		QueryPerformanceCounter(&eConstraints);
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		std::wstring str = std::to_wstring(static_cast<float>(eConstraints.QuadPart - sConstraints.QuadPart) * 1000.0f / static_cast<float>(freq.QuadPart));
		OutputDebugString(str.append(L" ms\n").c_str());
#endif

		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			// Update velocity and position
			m_velocities[v] = (p[v] - x[v]) / deltaTime;
			XMStoreFloat3(&m_vertices[v].position, p[v]);
		}

		// Update vertex buffer
		{
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, m_vertices, sizeof(m_vertices));
			m_vertexBuffer->Unmap(0, nullptr);
		}
	}

	UINT Cube::GetNumVertices(void) const
	{
		return NUM_VERTICES;
	}

	UINT Cube::GetNumIndices(void) const
	{
		return NUM_INDICES;
	}
}