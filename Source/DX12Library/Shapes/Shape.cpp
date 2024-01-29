#include "Shape.h"

namespace DX12Library
{
	Shape::Shape(void)
		: m_vertexBufferView()
		, m_indexBufferView()
	{
	}

	D3D12_VERTEX_BUFFER_VIEW& Shape::GetVertexBufferView(void)
	{
		return m_vertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW& Shape::GetIndexBufferView(void)
	{
		return m_indexBufferView;
	}

	const XMMATRIX& Shape::GetWorldMatrix(void) const
	{
		return m_world;
	}
}