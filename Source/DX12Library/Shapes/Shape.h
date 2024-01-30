#pragma once

#include "Common.h"
#include "DXSampleHelper.h"

namespace DX12Library
{
	class Shape
	{
	public:
		Shape(void);
		virtual ~Shape() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView();

		virtual VertexPosColor* GetVertices(void) = 0;
		virtual const WORD* GetIndices(void) const = 0;
		virtual UINT GetNumVertices(void) const = 0;
        virtual UINT GetNumIndices(void) const = 0;

		const XMMATRIX& GetWorldMatrix(void) const;

	protected:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		XMMATRIX m_world = XMMatrixIdentity();
	};
}