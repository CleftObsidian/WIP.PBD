#include "Sphere.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace DX12Library
{
	std::unique_ptr<Assimp::Importer> Sphere::sm_pImporter = std::make_unique<Assimp::Importer>();

	Sphere::Sphere(_In_ XMVECTOR& position)
		: Shape()
		, m_x(position)
		, m_radius(1.0f)
		, m_pScene(nullptr)
	{
		m_world *= XMMatrixTranslationFromVector(position);
	}

	void Sphere::Initialize(_In_ ID3D12Device* pDevice, _In_ ID3D12GraphicsCommandList* pCommandList)
	{
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
						//const aiVector3D& normal = pMesh->mNormals[j];
						//const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][j] : zero3d;

						VertexPosColor vertex =
						{
							.position = XMFLOAT3(position.x, position.y, position.z),
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

		const UINT vertexBufferSize = static_cast<UINT>(sizeof(VertexPosColor) * m_aVertices.size());
		{
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			// create default heap
			// default heap is memory on the GPU. Only the GPU has access to this memory
			// To get data into this heap, we will have to upload the data using
			// an upload heap
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties, // a default heap
				D3D12_HEAP_FLAG_NONE, // no flags
				&resourceDesc, // resource description for a buffer
				D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data from the upload heap to this heap
				nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
				IID_PPV_ARGS(&m_vertexBuffer)));
			m_vertexBuffer->SetName(L"Sphere Vertex Buffer");
		}

		{
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			// create upload heap
			// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
			// We will upload the vertex buffer using this heap to the default heap
			ID3D12Resource* vBufferUploadHeap;
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties, // upload heap
				D3D12_HEAP_FLAG_NONE, // no flags
				&resourceDesc, // resource description for a buffer
				D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
				nullptr,
				IID_PPV_ARGS(&vBufferUploadHeap)));

			// store vertex buffer in upload heap
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = m_aVertices.data();
			vertexData.RowPitch = vertexBufferSize;
			vertexData.SlicePitch = vertexBufferSize;

			// we are now creating a command with the command list to copy the data from
			// the upload heap to the default heap
			UpdateSubresources(pCommandList, m_vertexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &vertexData);

			// transition the vertex buffer data from copy destination state to vertex buffer state
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			pCommandList->ResourceBarrier(1, &barrier);
		}

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		UINT indexBufferSize = static_cast<UINT>(sizeof(WORD) * m_aIndices.size());
		// Create the index buffer.
		{
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			// create default heap to hold index buffer
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties, // a default heap
				D3D12_HEAP_FLAG_NONE, // no flags
				&resourceDesc, // resource description for a buffer
				D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
				nullptr, // optimized clear value must be null for this type of resource
				IID_PPV_ARGS(&m_indexBuffer)));
			m_indexBuffer->SetName(L"Sphere Index Buffer");
		}

		// create upload heap to upload index buffer
		{
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			ID3D12Resource* iBufferUploadHeap;
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProperties, // upload heap
				D3D12_HEAP_FLAG_NONE, // no flags
				&resourceDesc, // resource description for a buffer
				D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
				nullptr,
				IID_PPV_ARGS(&iBufferUploadHeap)));

			// store vertex buffer in upload heap
			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = m_aIndices.data(); // pointer to our index array
			indexData.RowPitch = indexBufferSize; // size of all our index buffer
			indexData.SlicePitch = indexBufferSize; // also the size of our index buffer

			// we are now creating a command with the command list to copy the data from
			// the upload heap to the default heap
			UpdateSubresources(pCommandList, m_indexBuffer.Get(), iBufferUploadHeap, 0, 0, 1, &indexData);

			// transition the vertex buffer data from copy destination state to vertex buffer state
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			pCommandList->ResourceBarrier(1, &barrier);
		}

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		m_indexBufferView.SizeInBytes = indexBufferSize;
	}

	void Sphere::Update(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	VertexPosColor* Sphere::GetVertices(void)
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
			XMVECTOR dp = lambda * collisionNormal * 0.5f;

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
			float sFric = std::sqrtf(this->FRICTION_S * collideSphere->FRICTION_S);
			float kFric = std::sqrtf(this->FRICTION_K * collideSphere->FRICTION_K);
			if (disLength < sFric * -C)
			{
				this->m_p -= 0.5f * displacement;
				collideSphere->m_p += 0.5f * displacement;
			}
			else
			{
				XMVECTOR delta = 0.5f * displacement * std::min(kFric * -C / disLength, 1.0f);
				this->m_p -= delta;
				collideSphere->m_p += delta;
			}
		}
	}

	void Sphere::SolveFloorConstraint(void)
	{
		if (-10.0f <= XMVectorGetX(m_p) && XMVectorGetX(m_p) <= 10.0f)
		{
			if (-10.0f <= XMVectorGetZ(m_p) && XMVectorGetZ(m_p) <= 10.0f)
			{
				// Solve floor(limited y-height) constraint
				if (XMVectorGetY(m_p) - m_radius < 0.0f)
				{
					// C(p) = p_y - radius >= 0
					XMVECTOR gradC = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
					float lambda = -(XMVectorGetY(m_p) - m_radius);	// lambda = -C(p) / |gradC|^2
					XMVECTOR dp = lambda * gradC;

					m_p += dp;
					
					// Friction
					XMVECTOR displacement = m_p - m_x;
					displacement -= XMVectorGetX(XMVector3Dot(displacement, gradC)) * gradC;
					float disLength = XMVectorGetX(XMVector3Length(displacement));
					if (disLength < FLT_EPSILON)
					{
						return;
					}
					if (disLength < (FRICTION_S * lambda))
					{
						m_p -= displacement;
					}
					else
					{
						m_p -= displacement * std::min(FRICTION_K * lambda / disLength, 1.0f);
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