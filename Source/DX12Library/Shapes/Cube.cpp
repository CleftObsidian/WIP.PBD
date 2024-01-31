#include "Cube.h"

namespace DX12Library
{
	Cube::Cube(_In_ XMVECTOR& position)
		: Shape()
		, m_restLengths()
		, m_p()
		, m_x()
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
		UNREFERENCED_PARAMETER(deltaTime);

		// Update vertex buffer
		{
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, m_vertices, sizeof(m_vertices));
			m_vertexBuffer->Unmap(0, nullptr);
		}
	}

	VertexPosColor* Cube::GetVertices(void)
	{
		return m_vertices;
	}

	const WORD* Cube::GetIndices(void) const
	{
		return ms_indicies;
	}

	UINT Cube::GetNumVertices(void) const
	{
		return NUM_VERTICES;
	}

	UINT Cube::GetNumIndices(void) const
	{
		return NUM_INDICES;
	}

	void Cube::PredictPosition(_In_ FLOAT deltaTime)
	{
		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			// Get position vector(x)
			m_x[v] = XMLoadFloat3(&m_vertices[v].position);

			// Estimate next position(p) only considering gravity (Euler Method)
			m_velocities[v] += deltaTime * GRAVITY;				// v <- v + dt * (gravity acceleration)
			m_p[v] = m_x[v] + (deltaTime * m_velocities[v]);	// p <- x + dt * v
		}
	}

	void Cube::SolveSelfDistanceConstraints(void)
	{
		// Solve distance constraints between vertices
		for (size_t idx = 1; idx < NUM_INDICES; ++idx)
		{
			// C(p1, p2) = |p1 - p2| - (rest length) = 0
			XMVECTOR edge = m_p[ms_indicies[idx - 1]] - m_p[ms_indicies[idx]];		// p1 - p2
			float distance = XMVectorGetX(XMVector3Length(edge));					// |p1 - p2|
			XMVECTOR normal = XMVector3Normalize(edge);								// (p1 - p2) / |p1 - p2|

			m_p[ms_indicies[idx - 1]] -= 0.5f * (distance - m_restLengths[idx]) * normal;
			m_p[ms_indicies[idx]] += 0.5f * (distance - m_restLengths[idx]) * normal;
		}
	}

	void Cube::SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape)
	{
		Cube* collideCube = static_cast<Cube*>(collideShape.get());
		XMVECTOR* other_p = collideCube->GetPositionPredictions();

		XMFLOAT3 thisCubePoints[NUM_VERTICES];
		XMFLOAT3 collideCubePoints[NUM_VERTICES];
		for (size_t i = 0; i < NUM_VERTICES; ++i)
		{
			XMStoreFloat3(&thisCubePoints[i], m_p[i]);
			XMStoreFloat3(&collideCubePoints[i], other_p[i]);
		}

		BoundingOrientedBox thisOBB;
		BoundingOrientedBox::CreateFromPoints(thisOBB, NUM_VERTICES, thisCubePoints, sizeof(XMFLOAT3));
		//thisOBB.Extents = { 1.0f, 1.0f, 1.0f };
		thisOBB.Extents = { 1.0f + FLT_EPSILON, 1.0f + FLT_EPSILON, 1.0f + FLT_EPSILON };

		BoundingOrientedBox otherOBB;
		BoundingOrientedBox::CreateFromPoints(otherOBB, collideCube->GetNumVertices(), collideCubePoints, sizeof(XMFLOAT3));
		//otherOBB.Extents = { 1.0f, 1.0f, 1.0f };
		otherOBB.Extents = { 1.0f + FLT_EPSILON, 1.0f + FLT_EPSILON, 1.0f + FLT_EPSILON };

		if (true == thisOBB.Intersects(otherOBB))
		{
			XMVECTOR thisCenter = XMLoadFloat3(&thisOBB.Center);
			XMVECTOR otherCenter = XMLoadFloat3(&otherOBB.Center);
			XMVECTOR centerToOtherCenter = otherCenter - thisCenter;
			float centerDistance = XMVectorGetX(XMVector3Length(centerToOtherCenter));
			centerToOtherCenter = XMVector3Normalize(centerToOtherCenter);

			float thisCenterToOtherOBBDistance;
			otherOBB.Intersects(thisCenter, centerToOtherCenter, thisCenterToOtherOBBDistance);
			float otherCenterToThisOBBDistance;
			thisOBB.Intersects(otherCenter, -centerToOtherCenter, otherCenterToThisOBBDistance);

			float intersectDistance = centerDistance - thisCenterToOtherOBBDistance - otherCenterToThisOBBDistance;

#ifdef _DEBUG
			OutputDebugString(std::to_wstring(thisCenterToOtherOBBDistance).c_str());
			OutputDebugString(L"\n");
			OutputDebugString(std::to_wstring(otherCenterToThisOBBDistance).c_str());
			OutputDebugString(L"\n");
			OutputDebugString(std::to_wstring(intersectDistance).c_str());
			OutputDebugString(L"\n\n");
#endif //_DEBUG

			XMVECTOR dp = -intersectDistance * centerToOtherCenter;
			if (FLT_EPSILON <= intersectDistance)
			{
				for (size_t v = 0; v < NUM_VERTICES; ++v)
				{
					if (CONTAINS == otherOBB.Contains(m_p[v]))
					{
						m_p[v] += dp * 0.5f / static_cast<float>(NUM_VERTICES);

						//m_p[v] += dp * 0.5f;
					}
					if (CONTAINS == thisOBB.Contains(other_p[v]))
					{
						other_p[v] -= dp * 0.5f / static_cast<float>(collideCube->GetNumVertices());

						//other_p[v] -= dp * 0.5f;
					}
				}
			}
			else
			{
				for (size_t v = 0; v < NUM_VERTICES; ++v)
				{
					if (CONTAINS == otherOBB.Contains(m_p[v]))
					{
						m_p[v] = m_x[v];
					}
					if (CONTAINS == thisOBB.Contains(other_p[v]))
					{
						//other_p[v] = collideCube->GetPositionsBeforeUpdate()[v];
					}
				}
			}
		}
	}

	void Cube::SolveFloorConstraint(void)
	{
		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			// Solve floor(limited y-height) constraint
			if (XMVectorGetY(m_p[v]) < 0.0f)
			{
				// C(p) = p_y >= 0
				XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
				float lambda = -XMVectorGetY(m_p[v]);	// lambda = -C(p) / |gradC|^2
				XMVECTOR dp = lambda * gradC;

				m_p[v] += dp;
			}
		}
	}

	void Cube::UpdateVertices(_In_ FLOAT deltaTime)
	{
		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			// Update velocity and position
			m_velocities[v] = (m_p[v] - m_x[v]) / deltaTime;
			XMStoreFloat3(&m_vertices[v].position, m_p[v]);
		}
	}

	XMVECTOR* Cube::GetPositionPredictions(void)
	{
		return m_p;
	}

	DirectX::XMVECTOR* Cube::GetPositionsBeforeUpdate(void)
	{
		return m_x;
	}
}