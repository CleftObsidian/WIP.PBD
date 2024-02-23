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
		RigidBodyShape(const XMVECTOR& position, const XMVECTOR& rotation, const XMVECTOR& scale, float mass, const std::vector<Collider>& colliders,
			float staticFrictionCoefficient, float dynamicFrictionCoefficient, float restitutionCoefficient, bool bIsFixed);
		RigidBodyShape(const RigidBodyShape& other) = delete;
		virtual ~RigidBodyShape();

		virtual void Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;

		virtual D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(void) = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(void) = 0;

		virtual UINT GetNumIndicesForRendering(void) const = 0;

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