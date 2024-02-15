#include "Cube.h"

namespace DX12Library
{
	ComPtr<ID3D12Resource> Cube::m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW Cube::m_indexBufferView;

	Cube::Cube(_In_ const XMVECTOR& position)
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
			m_restLengths[i] = XMVectorGetX(XMVector3Length(pos[ms_indices[i - 1]] - pos[ms_indices[i]]));
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
			m_vertexBuffer->SetName(L"Cube Vertex Buffer");

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		{
			const UINT indexBufferSize = sizeof(ms_indices);

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
			memcpy(pIndexDataBegin, ms_indices, indexBufferSize);
			m_indexBuffer->Unmap(0, nullptr);
			m_indexBuffer->SetName(L"Cube Index Buffer");

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

	D3D12_VERTEX_BUFFER_VIEW& Cube::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& Cube::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	Vertex* Cube::GetVertices(void)
	{
		return m_vertices;
	}

	const WORD* Cube::GetIndices(void) const
	{
		return ms_indices;
	}

	UINT Cube::GetNumVertices(void) const
	{
		return NUM_VERTICES;
	}

	UINT Cube::GetNumIndices(void) const
	{
		return NUM_INDICES;
	}

	bool Cube::CheckCollision(const std::shared_ptr<DX12Library::Shape> collideShape) const
	{
		return false;
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
		}

		DampVelocities();

		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			m_p[v] = m_x[v] + (deltaTime * m_velocities[v]);	// p <- x + dt * v
		}
	}

	void Cube::SolveSelfDistanceConstraints(void)
	{
		// Solve distance constraints between vertices
		for (size_t idx = 1; idx < NUM_INDICES; ++idx)
		{
			// C(p1, p2) = |p1 - p2| - (rest length) = 0
			XMVECTOR edge = m_p[ms_indices[idx - 1]] - m_p[ms_indices[idx]];		// p1 - p2
			float distance = XMVectorGetX(XMVector3Length(edge));					// |p1 - p2|
			XMVECTOR normal = XMVector3Normalize(edge);								// (p1 - p2) / |p1 - p2|

			m_p[ms_indices[idx - 1]] -= 0.5f * (distance - m_restLengths[idx]) * normal;
			m_p[ms_indices[idx]] += 0.5f * (distance - m_restLengths[idx]) * normal;
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
		thisOBB.Extents = { 1.0f, 1.0f, 1.0f };

		BoundingOrientedBox otherOBB;
		BoundingOrientedBox::CreateFromPoints(otherOBB, NUM_VERTICES, collideCubePoints, sizeof(XMFLOAT3));
		otherOBB.Extents = { 1.0f, 1.0f, 1.0f };

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

			float intersectDistance = std::_Float_abs(centerDistance - thisCenterToOtherOBBDistance - otherCenterToThisOBBDistance);

#ifdef _DEBUG
			OutputDebugString(std::to_wstring(thisCenterToOtherOBBDistance).c_str());
			OutputDebugString(L"\n");
			OutputDebugString(std::to_wstring(otherCenterToThisOBBDistance).c_str());
			OutputDebugString(L"\n");
			OutputDebugString(std::to_wstring(intersectDistance).c_str());
			OutputDebugString(L"\n\n");
#endif //_DEBUG

			XMVECTOR dp = -intersectDistance * centerToOtherCenter * 0.5f;

			for (size_t v = 0; v < NUM_VERTICES; ++v)
			{
				XMVECTOR ray = m_p[v] - thisCenter;
				float rayLength = XMVectorGetX(XMVector3Length(ray));
				ray = XMVector3Normalize(ray);

				float distance = -1.0f;
				bool bRayIntersected = otherOBB.Intersects(thisCenter, ray, distance);
				ContainmentType eContain = otherOBB.Contains(m_p[v]);

				if ((true == bRayIntersected || CONTAINS == eContain) && 0.0f <= distance && distance <= rayLength)
				{
					m_p[v] += dp;
				}

				XMVECTOR otherRay = other_p[v] - otherCenter;
				rayLength = XMVectorGetX(XMVector3Length(otherRay));
				otherRay = XMVector3Normalize(otherRay);

				float otherDistance = -1.0f;
				bool bOtherRayIntersected = thisOBB.Intersects(otherCenter, otherRay, otherDistance);
				eContain = otherOBB.Contains(other_p[v]);

				if ((true == bOtherRayIntersected || CONTAINS == eContain) && 0.0f <= otherDistance && otherDistance <= rayLength)
				{
					other_p[v] -= dp;
				}
			}

			// Friction
			//for (size_t v = 0; v < NUM_VERTICES; ++v)
			//{
			//	XMVECTOR displacement = (this->m_p[v] - this->m_x[v]) - (collideCube->m_p[v] - collideCube->m_x[v]);
			//	displacement -= XMVectorGetX(XMVector3Dot(displacement, -centerToOtherCenter)) * -centerToOtherCenter;
			//	float disLength = XMVectorGetX(XMVector3Length(displacement));
			//	if (disLength < FLT_EPSILON)
			//	{
			//		return;
			//	}
			//	float sFric = sqrtf(this->FRICTION_S * collideCube->FRICTION_S);
			//	float kFric = sqrtf(this->FRICTION_K * collideCube->FRICTION_K);
			//	if (disLength < sFric * intersectDistance)
			//	{
			//		this->m_p[v] -= 0.5f * displacement;
			//		collideCube->m_p[v] += 0.5f * displacement;
			//	}
			//	else
			//	{
			//		XMVECTOR delta = 0.5f * displacement * fminf(kFric * intersectDistance / disLength, 1.0f);
			//		this->m_p[v] -= delta;
			//		collideCube->m_p[v] += delta;
			//	}
			//}
		}
	}

	void Cube::SolveFloorConstraint(void)
	{
		// for all vertices
		for (size_t v = 0; v < NUM_VERTICES; ++v)
		{
			if (-10.0f < XMVectorGetX(m_p[v]) && XMVectorGetX(m_p[v]) < 10.0f)
			{
				if (-10.0f < XMVectorGetZ(m_p[v]) && XMVectorGetZ(m_p[v]) < 10.0f)
				{
					// Solve floor(limited y-height) constraint
					if (XMVectorGetY(m_p[v]) < 0.0f)
					{
						// C(p) = p_y >= 0
						XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
						float lambda = -XMVectorGetY(m_p[v]);	// lambda = -C(p) / |gradC|^2
						XMVECTOR dp = lambda * gradC;

						m_p[v] += dp;

						// Friction
						XMVECTOR displacement = m_p[v] - m_x[v];
						displacement -= XMVectorGetX(XMVector3Dot(displacement, gradC)) * gradC;
						float disLength = XMVectorGetX(XMVector3Length(displacement));
						if (disLength < FLT_EPSILON)
						{
							return;
						}
						if (disLength < (sqrtf(FRICTION_S) * lambda))
						{
							m_p[v] -= displacement;
						}
						else
						{
							m_p[v] -= displacement * fminf(sqrtf(FRICTION_K) * lambda / disLength, 1.0f);
						}
					}
				}
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

	void Cube::DampVelocities(void)
	{
		XMVECTOR x_cm = XMVectorZero();
		XMVECTOR v_cm = XMVectorZero();
		for (size_t i = 0; i < NUM_VERTICES; ++i)
		{
			x_cm += m_x[i];
			v_cm += m_velocities[i];
		}
		x_cm /= static_cast<float>(NUM_VERTICES);
		v_cm /= static_cast<float>(NUM_VERTICES);

		XMVECTOR r[NUM_VERTICES];

		XMVECTOR L = XMVectorZero();
		XMMATRIX I;
		I.r[0] = XMVectorZero();
		I.r[1] = XMVectorZero();
		I.r[2] = XMVectorZero();
		I.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMMATRIX r_tilde = XMMatrixIdentity();
		for (size_t i = 0; i < NUM_VERTICES; ++i)
		{
			r[i] = m_x[i] - x_cm;

			L += XMVector3Cross(r[i], m_velocities[i]);

			/*
			XMMATRIX r_tilde(0.0f, -XMVectorGetZ(r[i]), XMVectorGetY(r[i]), 0.0f,
				XMVectorGetZ(r[i]), 0.0f, -XMVectorGetX(r[i]), 0.0f,
				-XMVectorGetY(r[i]), XMVectorGetX(r[i]), 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f);
			*/
			
			r_tilde.r[0] = XMVectorSet(					0.0f,	 -XMVectorGetZ(r[i]),	  XMVectorGetY(r[i]), 0.0f);
			r_tilde.r[1] = XMVectorSet(   XMVectorGetZ(r[i]),					0.0f,	 -XMVectorGetX(r[i]), 0.0f);
			r_tilde.r[2] = XMVectorSet(  -XMVectorGetY(r[i]),	  XMVectorGetX(r[i]),					0.0f, 0.0f);
			r_tilde.r[3] = XMVectorZero();//XMVectorSet(					0.0f,					0.0f,					0.0f, 1.0f);
			
			XMMATRIX r_tilde_transpose = XMMatrixTranspose(r_tilde);
			I += r_tilde * r_tilde_transpose;
		}

		XMMATRIX invI = XMMatrixInverse(nullptr, I);
		if (true == XMMatrixIsInfinite(invI) || true == XMMatrixIsNaN(invI))
		{
			OutputDebugString(L"Invalid matrix");
			return;
		}
		XMVECTOR angularVelocity = XMVector3Transform(L, invI);

		for (size_t i = 0; i < NUM_VERTICES; ++i)
		{
			XMVECTOR dv = v_cm + XMVector3Cross(angularVelocity, r[i]) - m_velocities[i];
			m_velocities[i] += dv;
		}
	}

	XMVECTOR* Cube::GetPositionPredictions(void)
	{
		return m_p;
	}

	XMVECTOR* Cube::GetPositionsBeforeUpdate(void)
	{
		return m_x;
	}
}