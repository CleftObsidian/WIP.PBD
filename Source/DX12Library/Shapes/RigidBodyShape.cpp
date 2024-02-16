#include "RigidBodyShape.h"

namespace DX12Library
{
	RigidBodyShape::RigidBodyShape(_In_ const XMVECTOR& position, _In_ const XMVECTOR& rotation, _In_ const XMVECTOR& scale, _In_ float mass, _In_ std::vector<Collider>& colliders,
		_In_ float staticFrictionCoefficient, _In_ float dynamicFrictionCoefficient, _In_ float restitutionCoefficient, bool bIsFixed)
		: worldPosition(position)
		, worldRotation(rotation)
		, worldScale(scale)
		, colliders(std::move(colliders))
		, boundingSphereRadius()
		, forces()
		, inverseMass()
		//, inertiaTensor()
		//, inverseInertiaTensor()
		, linearVelocity(XMVectorZero())
		, angularVelocity(XMVectorZero())
		, bFixed(bIsFixed)
		, bActive(true)
		, deactivationTime(0.0f)
		, staticFrictionCoefficient(staticFrictionCoefficient)
		, dynamicFrictionCoefficient(dynamicFrictionCoefficient)
		, restitutionCoefficient(restitutionCoefficient)
		, prevWorldPosition()
		, prevWorldRotation()
		, prevLinearVelocity(XMVectorZero())
		, prevAngularVelocity(XMVectorZero())
	{
		boundingSphereRadius = GetCollidersBoundingSphereRadius(colliders);

		//if (true == bIsFixed)
		//{
		//	inverseMass = 0.0f;
		//	inertiaTensor.r[0] = XMVectorZero();
		//	inertiaTensor.r[1] = XMVectorZero();
		//	inertiaTensor.r[2] = XMVectorZero();
		//	inertiaTensor.r[3] = XMVectorZero();
		//	inverseInertiaTensor.r[0] = XMVectorZero();
		//	inverseInertiaTensor.r[1] = XMVectorZero();
		//	inverseInertiaTensor.r[2] = XMVectorZero();
		//	inverseInertiaTensor.r[3] = XMVectorZero();
		//}
		//else
		//{
		//	inverseMass = 1.0f / mass;
		//	inertiaTensor = GetCollidersDefaultInertiaTensor(colliders, mass);
		//	inverseInertiaTensor = XMMatrixInverse(nullptr, inertiaTensor);
		//	assert(false == XMMatrixIsInfinite(inverseInertiaTensor) && false == XMMatrixIsNaN(inverseInertiaTensor));
		//}

		assert(0.0f <= staticFrictionCoefficient && staticFrictionCoefficient <= 1.0f);
		assert(0.0f <= dynamicFrictionCoefficient && dynamicFrictionCoefficient <= 1.0f);
		assert(0.0f <= restitutionCoefficient && restitutionCoefficient <= 1.0f);
		if (staticFrictionCoefficient < dynamicFrictionCoefficient)
		{
			OutputDebugString(L"Warning: dynamic friction coefficient is greater than static friction coefficient\n");
		}
	}

	RigidBodyShape::~RigidBodyShape()
	{
	}

	const XMMATRIX RigidBodyShape::GetWorldMatrix(void) const
	{
		XMMATRIX worldTransform = XMMatrixIdentity();
		worldTransform *= XMMatrixTranslationFromVector(worldPosition);
		worldTransform *= XMMatrixRotationQuaternion(worldRotation);
		worldTransform *= XMMatrixScalingFromVector(worldScale);

		return worldTransform;
	}
}