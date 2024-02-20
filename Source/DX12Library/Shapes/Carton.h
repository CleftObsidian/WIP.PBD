#pragma once

#include "RigidBodyShape.h"

struct aiScene;
struct aiMesh;

namespace Assimp
{
	class Importer;
}

namespace DX12Library
{
	class Carton : public RigidBodyShape
	{
	public:
		Carton(void) = delete;
		Carton(_In_ const XMVECTOR& position) {};
		virtual ~Carton() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void);
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void);

		virtual Vertex* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

		virtual void PredictPosition(_In_ FLOAT deltaTime);
		virtual void SolveSelfDistanceConstraints(void);
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::RigidBodyShape> collideShape);
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

		static constexpr float FRICTION_S = 0.1f;
		static constexpr float FRICTION_K = 0.03f;

		static constexpr float STIFFNESS = 0.1f;
		static float L_STIFFNESS;

		float m_mass;

		XMVECTOR m_x = XMVectorZero();
		XMVECTOR m_p = XMVectorZero();
		XMVECTOR m_velocity = XMVectorZero();

		XMVECTOR m_orientation = XMQuaternionIdentity();
		XMVECTOR m_angularVelocity = XMVectorZero();
		XMMATRIX m_inertiaTensor = XMMatrixIdentity();

		static const aiScene* m_pScene;
		static std::vector<BasicMeshEntry> m_aMeshes;
	};
}