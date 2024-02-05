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
	class Sphere : public Shape
	{
	public:
		Sphere(void) = delete;
		Sphere(_In_ XMVECTOR& position);
		~Sphere() = default;

		virtual void Initialize(_In_ ID3D12Device * pDevice, _In_ ID3D12GraphicsCommandList* pCommandList);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual VertexPosColor* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

		virtual void PredictPosition(_In_ FLOAT deltaTime);
		virtual void SolveSelfDistanceConstraints(void);
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape);
		virtual void SolveFloorConstraint(void);
		virtual void UpdateVertices(_In_ FLOAT deltaTime);

	private:
		static std::unique_ptr<Assimp::Importer> sm_pImporter;

		std::vector<VertexPosColor> m_aVertices;
		std::vector<WORD> m_aIndices;

		static constexpr float FRICTION_S = 0.04f;
		static constexpr float FRICTION_K = 0.04f;

		XMVECTOR m_x;
		XMVECTOR m_p = XMVectorZero();
		XMVECTOR m_velocity = XMVectorZero();
		float m_radius;

		const aiScene* m_pScene;
		std::vector<BasicMeshEntry> m_aMeshes;
	};
}