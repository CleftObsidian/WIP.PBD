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
	std::vector<Vertex> RigidBodySphere::sm_aVertices;
	std::vector<WORD> RigidBodySphere::sm_aIndices;
	const aiScene* RigidBodySphere::m_pScene = nullptr;
	std::vector<BasicMeshEntry> RigidBodySphere::m_aMeshes;

	RigidBodySphere::RigidBodySphere(const XMVECTOR& position, const XMVECTOR& rotation, const XMVECTOR& scale, float mass,	const std::vector<Collider>& colliders,
		float staticFrictionCoefficient, float dynamicFrictionCoefficient, float restitutionCoefficient, bool bIsFixed)
		: RigidBodyShape(position, rotation, scale, mass, colliders, staticFrictionCoefficient, dynamicFrictionCoefficient, restitutionCoefficient, bIsFixed)
	{
	}

	void RigidBodySphere::Initialize(_In_ ID3D12Device* pDevice)
	{
		if (nullptr == m_pScene)
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

				sm_aVertices.reserve(uNumVertices);
				sm_aIndices.reserve(uNumIndices);

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
							//.color = XMFLOAT3(position.x, position.y, position.z)
							.color = XMFLOAT3(0.000000000f, 0.501960814f, 0.000000000f)	// green
						};

						sm_aVertices.push_back(vertex);
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

						sm_aIndices.push_back(aIndices[0]);
						sm_aIndices.push_back(aIndices[1]);
						sm_aIndices.push_back(aIndices[2]);
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
			const UINT vertexBufferSize = sizeof(Vertex) * static_cast<UINT>(sm_aVertices.size());

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
			memcpy(pVertexDataBegin, &sm_aVertices[0], vertexBufferSize);
			m_vertexBuffer->Unmap(0, nullptr);
			m_vertexBuffer->SetName(L"RigidBodySphere Vertex Buffer");

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		{
			const UINT indexBufferSize = sizeof(WORD) * static_cast<UINT>(sm_aIndices.size());

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
			memcpy(pIndexDataBegin, &sm_aIndices[0], indexBufferSize);
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

	const Vertex* RigidBodySphere::GetVertices(void)
	{
		return sm_aVertices.data();
	}

	const WORD* RigidBodySphere::GetIndices(void)
	{
		return sm_aIndices.data();
	}

	UINT RigidBodySphere::GetNumVertices(void)
	{
		return static_cast<UINT>(sm_aVertices.size());
	}

	UINT RigidBodySphere::GetNumIndices(void)
	{
		return static_cast<UINT>(sm_aIndices.size());
	}

	UINT RigidBodySphere::GetNumIndicesForRendering(void) const
	{
		return static_cast<UINT>(sm_aIndices.size());
	}
}