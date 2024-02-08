#include "Sphere.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace DX12Library
{
	ComPtr<ID3D12Resource> Sphere::m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW Sphere::m_vertexBufferView;
	ComPtr<ID3D12Resource> Sphere::m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW Sphere::m_indexBufferView;
	std::unique_ptr<Assimp::Importer> Sphere::sm_pImporter = std::make_unique<Assimp::Importer>();
	std::vector<Vertex> Sphere::m_aVertices;
	std::vector<WORD> Sphere::m_aIndices;
	const aiScene* Sphere::m_pScene = nullptr;
	std::vector<BasicMeshEntry> Sphere::m_aMeshes;

	float Sphere::L_STIFFNESS = 1.0f - powf(1.0f - STIFFNESS, 1.0f / SOLVER_ITERATION);

	Sphere::Sphere(_In_ const XMVECTOR& position)
		: Shape()
		, m_x(position)
		, m_radius(0.51f)
	{
		m_world *= XMMatrixScaling(m_radius, m_radius, m_radius) * XMMatrixTranslationFromVector(position);
	}

	void Sphere::Initialize(_In_ ID3D12Device* pDevice)
	{
		if (!m_pScene)
		{
			// Read the 3D model file
			m_pScene = sm_pImporter->ReadFile(
				"Contents/Sphere/sphere.obj",
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
							.color = XMFLOAT3(0.000000000f, 0.501960814f, 0.000000000f)	// green
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
				OutputDebugString(L"Contents/Sphere/sphere.obj");
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
			m_vertexBuffer->SetName(L"Sphere Vertex Buffer");

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
			m_indexBuffer->SetName(L"Sphere Index Buffer");

			m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = indexBufferSize;
		}
	}

	void Sphere::Update(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	D3D12_VERTEX_BUFFER_VIEW& Sphere::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& Sphere::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	Vertex* Sphere::GetVertices(void)
	{
		return m_aVertices.data();
	}

	const WORD* Sphere::GetIndices(void) const
	{
		return m_aIndices.data();
	}

	UINT Sphere::GetNumVertices(void) const
	{
		return static_cast<UINT>(m_aVertices.size());
	}

	UINT Sphere::GetNumIndices(void) const
	{
		return static_cast<UINT>(m_aIndices.size());
	}

	void Sphere::PredictPosition(_In_ FLOAT deltaTime)
	{
		// Get position vector(x)
		XMVECTOR scale;		// not use
		XMVECTOR rotation;	// not use
		XMMatrixDecompose(&scale, &rotation, &m_x, m_world);

		// Estimate next position(p) only considering gravity (Euler Method)
		m_velocity += deltaTime * GRAVITY;		// v <- v + dt * (gravity acceleration)
		m_p = m_x + deltaTime * m_velocity;		// p <- x + dt * v
	}

	void Sphere::SolveSelfDistanceConstraints(void)
	{
		// Sphere doesn't need to solve self distance
	}

	void Sphere::SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape)
	{
		Sphere* collideSphere = static_cast<Sphere*>(collideShape.get());

		XMVECTOR centerToOtherCenter = collideSphere->m_p - this->m_p;
		float centerToOtherCenterDistance = XMVectorGetX(XMVector3Length(centerToOtherCenter));
		float sumRadius = collideSphere->m_radius + this->m_radius;
		if (centerToOtherCenterDistance < sumRadius)
		{
			XMVECTOR collisionNormal = -XMVector3Normalize(centerToOtherCenter);

			// C(p_i, p_j) = |x_ij| - (r_i + r_j) >= 0
			float C = centerToOtherCenterDistance - sumRadius;
			float lambda = -C / XMVectorGetX(XMVector3Dot(collisionNormal, collisionNormal));
			XMVECTOR dp = lambda * collisionNormal * 0.5f * L_STIFFNESS;

			this->m_p += dp;
			collideSphere->m_p -= dp;

			// Friction
			XMVECTOR displacement = (this->m_p - this->m_x) - (collideSphere->m_p - collideSphere->m_x);
			displacement -= XMVectorGetX(XMVector3Dot(displacement, collisionNormal)) * collisionNormal;
			float disLength = XMVectorGetX(XMVector3Length(displacement));
			if (disLength < FLT_EPSILON)
			{
				return;
			}
			float sFric = sqrtf(this->FRICTION_S * collideSphere->FRICTION_S);
			float kFric = sqrtf(this->FRICTION_K * collideSphere->FRICTION_K);
			if (disLength < sFric * -C)
			{
				this->m_p -= 0.5f * displacement * L_STIFFNESS;
				collideSphere->m_p += 0.5f * displacement * L_STIFFNESS;
			}
			else
			{
				XMVECTOR delta = 0.5f * displacement * fminf(kFric * -C / disLength, 1.0f);
				this->m_p -= delta * L_STIFFNESS;
				collideSphere->m_p += delta * L_STIFFNESS;
			}
		}
	}

	void Sphere::SolveFloorConstraint(void)
	{
		if (-10.0f < XMVectorGetX(m_p) && XMVectorGetX(m_p) < 10.0f)
		{
			if (-10.0f < XMVectorGetZ(m_p) && XMVectorGetZ(m_p) < 10.0f)
			{
				// Solve floor(limited y-height) constraint
				if (XMVectorGetY(m_p) - m_radius < 0.0f)
				{
					// C(p) = p_y - radius >= 0
					XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
					float lambda = -(XMVectorGetY(m_p) - m_radius);	// lambda = -C(p) / |gradC|^2
					XMVECTOR dp = lambda * gradC * L_STIFFNESS;

					m_p += dp;
					
					// Friction
					XMVECTOR displacement = m_p - m_x;
					displacement -= XMVectorGetX(XMVector3Dot(displacement, gradC)) * gradC;
					float disLength = XMVectorGetX(XMVector3Length(displacement));
					if (disLength < FLT_EPSILON)
					{
						return;
					}
					if (disLength < (sqrtf(FRICTION_S) * lambda))
					{
						m_p -= displacement * L_STIFFNESS;
					}
					else
					{
						m_p -= displacement * fminf(sqrtf(FRICTION_K) * lambda / disLength, 1.0f) * L_STIFFNESS;
					}
				}
			}
		}
	}

	void Sphere::UpdateVertices(_In_ FLOAT deltaTime)
	{
		// Update velocity and position
		m_velocity = (m_p - m_x) / deltaTime;
		m_world *= XMMatrixTranslationFromVector(m_p - m_x);
	}
}