#include "Shape.h"

namespace DX12Library
{
	Shape::Shape(void)
	{
	}

	Shape::~Shape()
	{
	}

	const XMMATRIX& Shape::GetWorldMatrix(void) const
	{
		return m_world;
	}
}