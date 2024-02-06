#include "Plane.h"

namespace DX12Library
{
	ComPtr<ID3D12Resource> Plane::m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW Plane::m_vertexBufferView;
	ComPtr<ID3D12Resource> Plane::m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW Plane::m_indexBufferView;

	Plane::Plane(_In_ const XMVECTOR& position)
		: Shape()
	{
		m_world *= XMMatrixTranslationFromVector(position);
	}

	void Plane::Initialize(_In_ ID3D12Device* pDevice)
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
			m_vertexBuffer->SetName(L"Plane Vertex Buffer");

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
			m_indexBuffer->SetName(L"Plane Index Buffer");

			m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			m_indexBufferView.SizeInBytes = indexBufferSize;
		}
	}

	void Plane::Update(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	D3D12_VERTEX_BUFFER_VIEW& Plane::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& Plane::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	Vertex* Plane::GetVertices(void)
	{
		return m_vertices;
	}

	const WORD* Plane::GetIndices(void) const
	{
		return ms_indices;
	}

	UINT Plane::GetNumVertices(void) const
	{
		return NUM_VERTICES;
	}

	UINT Plane::GetNumIndices(void) const
	{
		return NUM_INDICES;
	}

	void Plane::PredictPosition(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}

	void Plane::SolveSelfDistanceConstraints(void)
	{

	}

	void Plane::SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape)
	{
		UNREFERENCED_PARAMETER(collideShape);
	}

	void Plane::SolveFloorConstraint(void)
	{

	}

	void Plane::UpdateVertices(_In_ FLOAT deltaTime)
	{
		UNREFERENCED_PARAMETER(deltaTime);
	}
}