#pragma once

#include "Shape.h"

struct aiScene;
struct aiMesh;

namespace Assimp
{
	class Importer;
}

namespace DX12Library
{
	class Carton : public Shape
	{
	public:
		Carton(void) = delete;
		Carton(_In_ const XMVECTOR& position) {};
		virtual ~Carton() = default;

	private:
	};
}