#pragma once

#include "Common.h"
#include "DXSampleHelper.h"

namespace DX12Library
{
	class RigidBodyShape
	{
	public:
		RigidBodyShape(void);
		virtual ~RigidBodyShape();

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void) = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void) = 0;

		virtual Vertex* GetVertices(void) = 0;
		virtual const WORD* GetIndices(void) const = 0;
		virtual UINT GetNumVertices(void) const = 0;
		virtual UINT GetNumIndices(void) const = 0;

		const XMMATRIX& GetWorldMatrix(void) const;

	protected:
		XMMATRIX m_world = XMMatrixIdentity();
	};
}