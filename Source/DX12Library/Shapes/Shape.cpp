#include "Shape.h"

namespace DX12Library
{
	Shape::Shape(_In_ XMVECTOR& position)
		: m_vertexBufferView()
		, m_indexBufferView()
	{
		m_world *= XMMatrixTranslationFromVector(position);
	}

	D3D12_VERTEX_BUFFER_VIEW& Shape::GetVertexBufferView()
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& Shape::GetIndexBufferView()
	{
		return m_indexBufferView;
	}

	const XMMATRIX& Shape::GetWorldMatrix() const
	{
		return m_world;
	}
}