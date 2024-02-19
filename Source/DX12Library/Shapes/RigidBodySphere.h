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
		RigidBodySphere(_In_ const XMVECTOR& position, _In_ const XMVECTOR& rotation, _In_ const XMVECTOR& scale, _In_ float mass, _In_ std::vector<Collider>& colliders,
			_In_ float staticFrictionCoefficient, _In_ float dynamicFrictionCoefficient, _In_ float restitutionCoefficient, bool bIsFixed);
		virtual ~RigidBodySphere() = default;

		virtual void Initialize(_In_ ID3D12Device* pDevice);
		virtual void Update(_In_ FLOAT deltaTime);

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void);
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void);

		virtual Vertex* GetVertices(void);
		virtual const WORD* GetIndices(void) const;
		virtual UINT GetNumVertices(void) const;
		virtual UINT GetNumIndices(void) const;

	public:
		static ComPtr<ID3D12Resource> m_vertexBuffer;
		static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		static ComPtr<ID3D12Resource> m_indexBuffer;
		static D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		static std::unique_ptr<Assimp::Importer> sm_pImporter;

		static std::vector<Vertex> m_aVertices;
		static std::vector<WORD> m_aIndices;

		static const aiScene* m_pScene;
		static std::vector<BasicMeshEntry> m_aMeshes;
	};
}