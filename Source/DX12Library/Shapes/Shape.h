#pragma once

#include "Common.h"
#include "DXSampleHelper.h"
#include <DirectXCollision.h>

namespace DX12Library
{
	class Shape
	{
	public:
		Shape(void);
		virtual ~Shape();

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void) = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void) = 0;

		virtual Vertex* GetVertices(void) = 0;
		virtual const WORD* GetIndices(void) const = 0;
		virtual UINT GetNumVertices(void) const = 0;
        virtual UINT GetNumIndices(void) const = 0;

		virtual bool CheckCollision(const std::shared_ptr<DX12Library::Shape> collideShape) const = 0;

		virtual void PredictPosition(_In_ FLOAT deltaTime) = 0;
		virtual void SolveSelfDistanceConstraints(void) = 0;
		virtual void SolveShapeCollision(std::shared_ptr<DX12Library::Shape> collideShape) = 0;
		virtual void SolveFloorConstraint(void) = 0;
		virtual void UpdateVertices(_In_ FLOAT deltaTime) = 0;

		const XMMATRIX& GetWorldMatrix(void) const;

	protected:
		XMMATRIX m_world = XMMatrixIdentity();
	};
}