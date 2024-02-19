#pragma once

#include "Common.h"
#include "DXSampleHelper.h"
#include "Physics/Collider.h"

struct PhysicsForce
{
	XMVECTOR position;
	XMVECTOR force;
	bool bIsLocalCoord;
};

namespace DX12Library
{
	class RigidBodyShape
	{
	public:
		RigidBodyShape(void) = delete;
		RigidBodyShape(_In_ const XMVECTOR& position, _In_ const XMVECTOR& rotation, _In_ const XMVECTOR& scale, _In_ float mass, _In_ std::vector<Collider>& colliders,
			_In_ float staticFrictionCoefficient, _In_ float dynamicFrictionCoefficient, _In_ float restitutionCoefficient, bool bIsFixed);
		virtual ~RigidBodyShape();

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void) = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void) = 0;

		virtual Vertex* GetVertices(void) = 0;
		virtual const WORD* GetIndices(void) const = 0;
		virtual UINT GetNumVertices(void) const = 0;
		virtual UINT GetNumIndices(void) const = 0;

		const XMMATRIX GetWorldMatrix(void) const;
		void AddForce(_In_ XMVECTOR position, _In_ XMVECTOR force, _In_ bool bIsLocalCoords);
		const XMMATRIX GetDynamicInertiaTensor(void) const;
		const XMMATRIX GetDynamicInverseInertiaTensor(void) const;

	public:
		size_t id;

		//XMMATRIX m_world = XMMatrixIdentity();
		XMVECTOR worldPosition;
		XMVECTOR worldRotation;
		XMVECTOR worldScale;

		// Physics
		std::vector<Collider> colliders;
		float boundingSphereRadius;
		std::vector<PhysicsForce> forces;
		float inverseMass;
		XMMATRIX inertiaTensor;
		XMMATRIX inverseInertiaTensor;
		XMVECTOR linearVelocity;
		XMVECTOR angularVelocity;
		bool bFixed;
		bool bActive;
		float deactivationTime;
		float staticFrictionCoefficient;
		float dynamicFrictionCoefficient;
		float restitutionCoefficient;
		
		// PBD support
		XMVECTOR prevWorldPosition;
		XMVECTOR prevWorldRotation;
		XMVECTOR prevLinearVelocity;
		XMVECTOR prevAngularVelocity;
	};
}