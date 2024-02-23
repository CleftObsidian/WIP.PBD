#include "RigidBodyCube.h"

namespace DX12Library
{
	ComPtr<ID3D12Resource> RigidBodyCube::m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW RigidBodyCube::m_vertexBufferView;
	ComPtr<ID3D12Resource> RigidBodyCube::m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW RigidBodyCube::m_indexBufferView;

	RigidBodyCube::RigidBodyCube(const XMVECTOR& position, const XMVECTOR& rotation, const XMVECTOR& scale, float mass, const std::vector<Collider>& colliders,
		float staticFrictionCoefficient, float dynamicFrictionCoefficient, float restitutionCoefficient, bool bIsFixed)
		: RigidBodyShape(position, rotation, scale, mass, colliders, staticFrictionCoefficient, dynamicFrictionCoefficient, restitutionCoefficient, bIsFixed)
	{
	}

	void RigidBodyCube::Initialize(_In_ ID3D12Device* pDevice)
	{
		{
			const UINT vertexBufferSize = sizeof(sm_vertices);

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
			memcpy(pVertexDataBegin, sm_vertices, vertexBufferSize);
			m_vertexBuffer->Unmap(0, nullptr);
			m_vertexBuffer->SetName(L"Cube Vertex Buffer");

			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		{
			const UINT indexBufferSize = sizeof(sm_indices);

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
			memcpy(pIndexDataBegin, sm_indices, indexBufferSize);
			m_indexBuffer->Unmap(0, nullptr);
			m_indexBuffer->SetName(L"Cube Index Buffer");

			m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = indexBufferSize;
		}
	}

	void RigidBodyCube::Update(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	D3D12_VERTEX_BUFFER_VIEW& RigidBodyCube::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& RigidBodyCube::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	const Vertex* RigidBodyCube::GetVertices(void)
	{
		return sm_vertices;
	}

	const WORD* RigidBodyCube::GetIndices(void)
	{
		return sm_indices;
	}

	UINT RigidBodyCube::GetNumVertices(void)
	{
		return NUM_VERTICES;
	}

	UINT RigidBodyCube::GetNumIndices(void)
	{
		return NUM_INDICES;
	}

	UINT RigidBodyCube::GetNumIndicesForRendering(void) const
	{
		return NUM_INDICES;
	}
}