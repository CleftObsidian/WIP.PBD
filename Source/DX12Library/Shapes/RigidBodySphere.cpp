#include "RigidBodySphere.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace DX12Library
{
	ComPtr<ID3D12Resource> RigidBodySphere::m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW RigidBodySphere::m_vertexBufferView;
	ComPtr<ID3D12Resource> RigidBodySphere::m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW RigidBodySphere::m_indexBufferView;
	std::unique_ptr<Assimp::Importer> RigidBodySphere::sm_pImporter = std::make_unique<Assimp::Importer>();
	std::vector<Vertex> RigidBodySphere::m_aVertices;
	std::vector<WORD> RigidBodySphere::m_aIndices;
	float RigidBodySphere::m_radius = 0.51f;
	const aiScene* RigidBodySphere::m_pScene = nullptr;
	std::vector<BasicMeshEntry> RigidBodySphere::m_aMeshes;

	RigidBodySphere::RigidBodySphere(_In_ const XMVECTOR& position)
		: Shape()
		, m_x(position)
	{
		m_world *= XMMatrixScaling(m_radius, m_radius, m_radius) * XMMatrixTranslationFromVector(position);
	}

	void RigidBodySphere::Initialize(_In_ ID3D12Device* pDevice)
	{
		if (!m_pScene)
		{
			// Read the 3D model file
			m_pScene = sm_pImporter->ReadFile(
				"Contents/Sphere/Sphere.obj",
				ASSIMP_LOAD_FLAGS
			);

			// Application is now responsible of deleting this scene
			m_pScene = sm_pImporter->GetOrphanedScene();

			// Initialize the model
			if (m_pScene)
			{
				m_aMeshes.resize(m_pScene->mNumMeshes);

				UINT uNumVertices = 0u;
				UINT uNumIndices = 0u;
				for (UINT i = 0u; i < m_pScene->mNumMeshes; ++i)
				{
					m_aMeshes[i].uNumIndices = m_pScene->mMeshes[i]->mNumFaces * 3u;
					m_aMeshes[i].uBaseVertex = uNumVertices;
					m_aMeshes[i].uBaseIndex = uNumIndices;

					uNumVertices += m_pScene->mMeshes[i]->mNumVertices;
					uNumIndices += m_aMeshes[i].uNumIndices;
				}

				m_aVertices.reserve(uNumVertices);
				m_aIndices.reserve(uNumIndices);

				for (UINT i = 0u; i < m_aMeshes.size(); ++i)
				{
					const aiMesh* pMesh = m_pScene->mMeshes[i];
					const aiVector3D zero3d(0.0f, 0.0f, 0.0f);

					// Populate the vertex attribute vector
					for (UINT j = 0u; j < pMesh->mNumVertices; ++j)
					{
						const aiVector3D& position = pMesh->mVertices[j];
						const aiVector3D& normal = pMesh->mNormals[j];
						//const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][j] : zero3d;

						Vertex vertex =
						{
							.position = XMFLOAT3(position.x, position.y, position.z),
							.normal = XMFLOAT3(normal.x, normal.y, normal.z),
							.color = XMFLOAT3(position.x, position.y, position.z)
							//.color = XMFLOAT3(0.000000000f, 0.501960814f, 0.000000000f)	// green
						};

						m_aVertices.push_back(vertex);
					}

					// Populate the index buffer
					for (UINT j = 0u; j < pMesh->mNumFaces; ++j)
					{
						const aiFace& face = pMesh->mFaces[j];
						assert(face.mNumIndices == 3u);

						WORD aIndices[3] =
						{
							static_cast<WORD>(face.mIndices[0]),
							static_cast<WORD>(face.mIndices[1]),
							static_cast<WORD>(face.mIndices[2]),
						};

						m_aIndices.push_back(aIndices[0]);
						m_aIndices.push_back(aIndices[1]);
						m_aIndices.push_back(aIndices[2]);
					}
				}
			}
			else
			{
				OutputDebugString(L"Error parsing ");
				OutputDebugString(L"Contents/Sphere/Sphere.obj");
				OutputDebugString(L": ");
				OutputDebugStringA(sm_pImporter->GetErrorString());
				OutputDebugString(L"\n");
			}
		}
		else
		{
			return;
		}

		{
			const UINT vertexBufferSize = sizeof(Vertex) * static_cast<UINT>(m_aVertices.size());

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)));

			void* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin));
			memcpy(pVertexDataBegin, &m_aVertices[0], vertexBufferSize);
			m_vertexBuffer->Unmap(0, nullptr);
			m_vertexBuffer->SetName(L"RigidBodySphere Vertex Buffer");

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		{
			const UINT indexBufferSize = sizeof(WORD) * static_cast<UINT>(m_aIndices.size());

			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_indexBuffer)));

			void* pIndexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_indexBuffer->Map(0, &readRange, &pIndexDataBegin));
			memcpy(pIndexDataBegin, &m_aIndices[0], indexBufferSize);
			m_indexBuffer->Unmap(0, nullptr);
			m_indexBuffer->SetName(L"RigidBodySphere Index Buffer");

			m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = indexBufferSize;
		}
	}

	void RigidBodySphere::Update(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	D3D12_VERTEX_BUFFER_VIEW& RigidBodySphere::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& RigidBodySphere::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	Vertex* RigidBodySphere::GetVertices(void)
	{
		return m_aVertices.data();
	}

	const WORD* RigidBodySphere::GetIndices(void) const
	{
		return m_aIndices.data();
	}

	UINT RigidBodySphere::GetNumVertices(void) const
	{
		return static_cast<UINT>(m_aVertices.size());
	}

	UINT RigidBodySphere::GetNumIndices(void) const
	{
		return static_cast<UINT>(m_aIndices.size());
	}

	bool RigidBodySphere::CheckCollision(const std::shared_ptr<DX12Library::RigidBodyShape> collideShape) const
	{
		RigidBodySphere* collideRigidBodySphere = static_cast<RigidBodySphere*>(collideShape.get());

		XMVECTOR centerToOtherCenter = collideRigidBodySphere->m_p - this->m_p;
		float centerToOtherCenterDistanceSq = XMVectorGetX(XMVector3LengthSq(centerToOtherCenter));
		float sumRadius = collideRigidBodySphere->m_radius + this->m_radius;
		if (centerToOtherCenterDistanceSq < sumRadius * sumRadius)
		{
			return true;
		}

		return false;
	}

	void RigidBodySphere::PredictPosition(_In_ FLOAT deltaTime)
	{
		// Get position vector(x)
		XMVECTOR scale;		// not use
		XMVECTOR rotation;	// not use
		XMMatrixDecompose(&scale, &rotation, &m_x, m_world);

		// Estimate next position(p) only considering gravity (Euler Method)
		m_velocity += deltaTime * GRAVITY;		// v <- v + dt * (gravity acceleration)
		m_p = m_x + deltaTime * m_velocity;		// p <- x + dt * v
	}

	void RigidBodySphere::SolveSelfDistanceConstraints(void)
	{
		// RigidBodySphere doesn't need to solve self distance
	}

	void RigidBodySphere::SolveShapeCollision(std::shared_ptr<DX12Library::RigidBodyShape> collideShape)
	{
		RigidBodySphere* collideRigidBodySphere = static_cast<RigidBodySphere*>(collideShape.get());

		XMVECTOR centerToOtherCenter = collideRigidBodySphere->m_p - this->m_p;
		float centerToOtherCenterDistance = XMVectorGetX(XMVector3Length(centerToOtherCenter));
		float sumRadius = collideRigidBodySphere->m_radius + this->m_radius;
		if (centerToOtherCenterDistance < sumRadius)
		{
			XMVECTOR collisionNormal = -XMVector3Normalize(centerToOtherCenter);

			// C(p_i, p_j) = |p_i - p_j| - (r_i + r_j) >= 0
			float C = centerToOtherCenterDistance - sumRadius;
			float dLambda = -C / (XMVectorGetX(XMVector3Dot(collisionNormal, collisionNormal)) + COMPLIANCE);
			dLambda *= 0.5f;
			XMVECTOR dp = dLambda * collisionNormal;

			this->m_p += dp;
			collideRigidBodySphere->m_p -= dp;

			// Friction
			XMVECTOR displacement = (this->m_p - this->m_x) - (collideRigidBodySphere->m_p - collideRigidBodySphere->m_x);
			displacement -= XMVectorGetX(XMVector3Dot(displacement, collisionNormal)) * collisionNormal;
			float disLength = XMVectorGetX(XMVector3Length(displacement));
			if (disLength < FLT_EPSILON)
			{
				return;
			}
			float sFric = (this->FRICTION_S + collideRigidBodySphere->FRICTION_S) * 0.5f;
			float kFric = (this->FRICTION_K + collideRigidBodySphere->FRICTION_K) * 0.5f;
			if (disLength < sFric * -C)
			{
				this->m_p -= 0.5f * displacement;
				collideRigidBodySphere->m_p += 0.5f * displacement;
			}
			else
			{
				XMVECTOR delta = 0.5f * displacement * fminf(kFric * -C / disLength, 1.0f);
				this->m_p -= delta;
				collideRigidBodySphere->m_p += delta;
			}
		}
	}

	void RigidBodySphere::SolveFloorConstraint(void)
	{
		if (-10.0f < XMVectorGetX(m_p) && XMVectorGetX(m_p) < 10.0f)
		{
			if (-10.0f < XMVectorGetZ(m_p) && XMVectorGetZ(m_p) < 10.0f)
			{
				// Solve floor(limited y-height) constraint
				if (XMVectorGetY(m_p) - m_radius < 0.0f)
				{
					// C(p) = p_y - radius >= 0
					float C = XMVectorGetY(m_p) - m_radius;
					XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
					// lambda = -C(p) / |gradC|^2
					float dLambda = -C / (XMVectorGetX(XMVector3Dot(gradC, gradC)) + COMPLIANCE);
					XMVECTOR dp = dLambda * gradC;

					m_p += dp;

					// Friction
					XMVECTOR displacement = m_p - m_x;
					displacement -= XMVectorGetX(XMVector3Dot(displacement, gradC)) * gradC;
					float disLength = XMVectorGetX(XMVector3Length(displacement));
					if (disLength < FLT_EPSILON)
					{
						return;
					}
					if (disLength < (FRICTION_S * dLambda))
					{
						m_p -= displacement;
					}
					else
					{
						m_p -= displacement * fminf(FRICTION_K * dLambda / disLength, 1.0f);
					}
				}
			}
		}
	}

	void RigidBodySphere::UpdateVertices(_In_ FLOAT deltaTime)
	{
		// Update velocity and position
		m_velocity = (m_p - m_x) / deltaTime;

		m_world *= XMMatrixTranslationFromVector(m_p - m_x);
	}
}