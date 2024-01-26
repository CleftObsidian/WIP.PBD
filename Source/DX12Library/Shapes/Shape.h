#pragma once

#include "Common.h"
#include "DXSampleHelper.h"

namespace DX12Library
{
	class Shape
	{
	public:
		Shape() = delete;
		Shape(_In_ XMVECTOR& position);
		virtual ~Shape() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView();

		virtual UINT GetNumVertices() const = 0;
        virtual UINT GetNumIndices() const = 0;

		const XMMATRIX& GetWorldMatrix() const;

	protected:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		XMMATRIX m_world = XMMatrixIdentity();
	};
}