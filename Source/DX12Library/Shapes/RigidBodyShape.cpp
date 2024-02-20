#include "RigidBodyShape.h"

namespace DX12Library
{
	RigidBodyShape::RigidBodyShape(const XMVECTOR& position, const XMVECTOR& rotation, const XMVECTOR& scale, float mass, const std::vector<Collider>& colliders,
		float staticFrictionCoefficient, float dynamicFrictionCoefficient, float restitutionCoefficient, bool bIsFixed)
		: id()
		, worldPosition(position)
		, worldRotation(rotation)
		, worldScale(scale)
		, colliders(colliders)
		, boundingSphereRadius()
		, forces()
		, inverseMass()
		, inertiaTensor()
		, inverseInertiaTensor()
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

		if (true == bIsFixed)
		{
			inverseMass = 0.0f;
			inertiaTensor.r[0] = XMVectorZero();
			inertiaTensor.r[1] = XMVectorZero();
			inertiaTensor.r[2] = XMVectorZero();
			inertiaTensor.r[3] = XMVectorZero();
			inverseInertiaTensor.r[0] = XMVectorZero();
			inverseInertiaTensor.r[1] = XMVectorZero();
			inverseInertiaTensor.r[2] = XMVectorZero();
			inverseInertiaTensor.r[3] = XMVectorZero();
		}
		else
		{
			inverseMass = 1.0f / mass;
			inertiaTensor = GetCollidersDefaultInertiaTensor(colliders, mass);
			inverseInertiaTensor = XMMatrixInverse(nullptr, inertiaTensor);
			assert(false == XMMatrixIsInfinite(inverseInertiaTensor) && false == XMMatrixIsNaN(inverseInertiaTensor));
		}

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

	// Add a force to an shape
	// If local_coords is false, then the position and force are represented in world coordinates, assuming that the center of the
	// world is the center of the entity. That is, the coordinate (0, 0, 0) corresponds to the center of the entity in world coords.
	// If local_coords is true, then the position and force are represented in local coords.
	void RigidBodyShape::AddForce(_In_ XMVECTOR position, _In_ XMVECTOR force, _In_ bool bIsLocalCoords)
	{
		if (true == bIsLocalCoords)
		{
			// If the force and position are in local cords, we first convert them to world coords
			// (actually, we convert them to ~"world coords centered at shape"~)
			force = XMVector3Rotate(force, worldRotation);

			// note that we don't need translation since we want to be centered at shape anyway
			XMMATRIX modelMatrixNoTranslation = XMMatrixScalingFromVector(worldScale) * XMMatrixRotationQuaternion(worldRotation);
			position = XMVector3Transform(position, modelMatrixNoTranslation);
		}

		PhysicsForce pf;
		pf.force = force;
		pf.position = position;
		
		forces.push_back(pf);
	}

	const XMMATRIX RigidBodyShape::GetDynamicInertiaTensor(void) const
	{
		XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(worldRotation);
		
		return rotationMatrix * inertiaTensor * XMMatrixTranspose(rotationMatrix);
	}

	const XMMATRIX RigidBodyShape::GetDynamicInverseInertiaTensor(void) const
	{
		XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(worldRotation);

		return rotationMatrix * inverseInertiaTensor * XMMatrixTranspose(rotationMatrix);
	}
}