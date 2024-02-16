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
		Sphere(_In_ const XMVECTOR& position);
		virtual ~Sphere() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void);
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void);

		virtual Vertex* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

		virtual bool CheckCollision(const std::shared_ptr<DX12Library::Shape> collideShape) const;

		virtual void PredictPosition(_In_ FLOAT deltaTime);
		virtual void SolveSelfDistanceConstraints(void);
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape);
		virtual void SolveFloorConstraint(void);
		virtual void UpdateVertices(_In_ FLOAT deltaTime);

	private:
		static ComPtr<ID3D12Resource> m_vertexBuffer;
		static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		static ComPtr<ID3D12Resource> m_indexBuffer;
		static D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		static std::unique_ptr<Assimp::Importer> sm_pImporter;

		static std::vector<Vertex> m_aVertices;
		static std::vector<WORD> m_aIndices;

		static constexpr float FRICTION_S = 0.74f;
		static constexpr float FRICTION_K = 0.57f;

		static constexpr float STIFFNESS = 2.0f * 100000000000.0f;	// metal stiffness = 2 * 10^11 N/m^2
		static constexpr float COMPLIANCE = 1.0f / STIFFNESS;		// inverse of stiffness

		XMVECTOR m_x;
		XMVECTOR m_p = XMVectorZero();
		XMVECTOR m_velocity = XMVectorZero();
		static float m_radius;

		static const aiScene* m_pScene;
		static std::vector<BasicMeshEntry> m_aMeshes;
	};
}