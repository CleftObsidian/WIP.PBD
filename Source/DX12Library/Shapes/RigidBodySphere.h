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
	class RigidBodySphere : public RigidBodyShape
	{
	public:
		RigidBodySphere(void) = delete;
		RigidBodySphere(const XMVECTOR& position, const XMVECTOR& rotation, const XMVECTOR& scale, float mass, const std::vector<Collider>& colliders,
			float staticFrictionCoefficient, float dynamicFrictionCoefficient, float restitutionCoefficient, bool bIsFixed);
		RigidBodySphere(const RigidBodySphere& other) = delete;
		virtual ~RigidBodySphere() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void);
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void);

		static const Vertex* GetVertices(void);
		static const WORD* GetIndices(void);
		static UINT GetNumVertices(void);
		static UINT GetNumIndices(void);

		virtual UINT GetNumIndicesForRendering(void) const;

	private:
		static ComPtr<ID3D12Resource> m_vertexBuffer;
		static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		static ComPtr<ID3D12Resource> m_indexBuffer;
		static D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		static std::unique_ptr<Assimp::Importer> sm_pImporter;

		static std::vector<Vertex> sm_aVertices;
		static std::vector<WORD> sm_aIndices;

		static const aiScene* m_pScene;
		static std::vector<BasicMeshEntry> m_aMeshes;
	};
}