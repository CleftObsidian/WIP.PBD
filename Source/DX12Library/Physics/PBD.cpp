#include "PBD.h"
#include "Broad.h"

static std::vector<Constraint>* copyConstraints(std::vector<Constraint>* constraints)
{
	if (nullptr == constraints)
	{
		return new std::vector<Constraint>;
	}

	std::vector<Constraint>* copiedConstraints = new std::vector<Constraint>;
	std::copy(constraints->begin(), constraints->end(), copiedConstraints->begin());

	for (size_t i = 0; i < copiedConstraints->size(); ++i)
	{
		Constraint* constraint = &copiedConstraints->at(i);

		// Reset lambda
		switch (constraint->type)
		{
		case ConstraintType::POSITIONAL_CONSTRAINT:
			constraint->positional_constraint.lambda = 0.0f;
			break;
		case ConstraintType::COLLISION_CONSTRAINT:
			constraint->collision_constraint.lambda_t = 0.0f;
			constraint->collision_constraint.lambda_n = 0.0f;
			break;
		}
	}

	return copiedConstraints;
}

static void clippingContactToCollisionConstraint(std::shared_ptr<DX12Library::RigidBodyShape> s1, std::shared_ptr<DX12Library::RigidBodyShape> s2,
	ColliderContact* contact, Constraint* constraint)
{
	constraint->type = ConstraintType::COLLISION_CONSTRAINT;
	constraint->s1_id = s1->id;
	constraint->s2_id = s2->id;
	constraint->collision_constraint.normal = contact->collision_normal;
	constraint->collision_constraint.lambda_t = 0.0f;
	constraint->collision_constraint.lambda_n = 0.0f;

	XMVECTOR r1_world = contact->collision_point1 - s1->worldPosition;
	XMVECTOR r2_world = contact->collision_point2 - s2->worldPosition;

	constraint->collision_constraint.r1_local = XMVector3InverseRotate(r1_world, s1->worldRotation);
	constraint->collision_constraint.r2_local = XMVector3InverseRotate(r2_world, s2->worldRotation);
}

static void solvePositionalConstraint(Constraint* constraint, float h, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes)
{
	assert(constraint->type == ConstraintType::POSITIONAL_CONSTRAINT);

	std::shared_ptr<DX12Library::RigidBodyShape> s1 = shapes[constraint->s1_id];
	std::shared_ptr<DX12Library::RigidBodyShape> s2 = shapes[constraint->s2_id];

	XMVECTOR attachmentDistance = s1->worldPosition - s2->worldPosition;
	XMVECTOR delta_x = attachmentDistance - constraint->positional_constraint.distance;

	// TODO
}

static void solveCollisionConstraint(Constraint* constraint, float h, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes)
{

}

static void solveConstraint(Constraint* constraint, float h, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes)
{
	switch (constraint->type)
	{
	case ConstraintType::POSITIONAL_CONSTRAINT:
		solvePositionalConstraint(constraint, h, shapes);
		break;
	case ConstraintType::COLLISION_CONSTRAINT:
		solveCollisionConstraint(constraint, h, shapes);
		break;
	}
	
	assert(false);
}

static void simulatePBDWithConstraints(float dt, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes,
	std::vector<Constraint>* externalConstraints, size_t numSubsteps, size_t numPosIters, bool bEnableCollision)
{
	if (dt <= 0.0f)
	{
		return;
	}

	float h = dt / static_cast<float>(numSubsteps);

	std::vector<BroadCollisionPair> broadCollisionPairs;
	GetBroadCollisionPairs(shapes, broadCollisionPairs);

	// Main loop of the PBD simulation
	for (size_t i = 0; i < numSubsteps; ++i)
	{
		std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>::iterator shape;
		for (shape = shapes.begin(); shape != shapes.end(); ++shape)
		{
			std::shared_ptr<DX12Library::RigidBodyShape> s = shape->second;

			// Store the previous position and orientation of the shape
			s->prevWorldPosition = s->worldPosition;
			s->prevWorldRotation = s->worldRotation;

			if (true == s->bFixed || false == s->bActive)
			{
				continue;
			}

			// Calculate the external force and torque of the shape
			XMVECTOR externalForce = XMVectorZero();
			XMVECTOR externalTorque = XMVectorZero();
			{
				const XMVECTOR centerOfMass = XMVectorZero();
				for (size_t i = 0; i < s->forces.size(); ++i)
				{
					externalForce += s->forces[i].force;

					XMVECTOR distance = s->forces[i].position - centerOfMass;
					externalTorque += XMVector3Cross(distance, s->forces[i].force);
				}
			}
			
			// Update the shape position and linear velocity based on the current velocity and applied forces
			s->linearVelocity += h * s->inverseMass * externalForce;
			s->worldPosition += h * s->linearVelocity;

			// Update the shape orientation and angular velocity based on the current velocity and applied torques
			s->angularVelocity += h * XMVector3Transform(externalTorque - XMVector3Cross(s->angularVelocity,
				XMVector3Transform(s->angularVelocity, s->GetDynamicInertiaTensor())), s->GetDynamicInverseInertiaTensor());
			XMVECTOR angularQ = XMVectorSet(XMVectorGetX(s->angularVelocity), XMVectorGetY(s->angularVelocity), XMVectorGetZ(s->angularVelocity), 0.0f);
			XMVECTOR q = XMQuaternionMultiply(angularQ, s->worldRotation);
			s->worldRotation += h * 0.5f * q;
			s->worldRotation = XMQuaternionNormalize(s->worldRotation);

			// Create the constraints array
			std::vector<Constraint>* constraints = copyConstraints(externalConstraints);

			// In each substep we need to check for collisions
			if (true == bEnableCollision)
			{
				for (size_t j = 0; j < broadCollisionPairs.size(); ++j)
				{
					std::shared_ptr<DX12Library::RigidBodyShape> s1 = shapes[broadCollisionPairs[j].s1_id];
					std::shared_ptr<DX12Library::RigidBodyShape> s2 = shapes[broadCollisionPairs[j].s2_id];

					// If e1 is "colliding" with e2, they must be either both active or both inactive
					if (!s1->bFixed && !s2->bFixed) {
						assert((s1->bActive && s2->bActive) || (!s1->bActive && !s2->bActive));
					}

					// No need to solve the collision if both entities are either inactive or fixed
					if ((s1->bFixed || !s1->bActive) && (s2->bFixed || !s2->bActive)) {
						continue;
					}

					UpdateColliders(s1->colliders, s1->worldPosition, s1->worldRotation);
					UpdateColliders(s2->colliders, s2->worldPosition, s2->worldRotation);

					std::vector<ColliderContact> contacts = GetCollidersContacts(s1->colliders, s2->colliders);
					for (size_t l = 0; l < contacts.size(); ++l)
					{
						ColliderContact* contact = &contacts[l];
						Constraint constraint;
						clippingContactToCollisionConstraint(s1, s2, contact, &constraint);
						constraints->push_back(constraint);
					}
				}
			}

			// Now we run the PBD solver with NUM_POS_ITERS iterations
			for (size_t j = 0; j < numPosIters; ++j)
			{
				for (size_t k = 0; k < constraints->size(); ++k)
				{
					Constraint* constraint = &constraints->at(k);
					solveConstraint(constraint, h, shapes);
				}
			}
		}
	}
}

void SimulatePBD(float dt, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes, size_t numSubsteps, size_t numPosIters, bool bEnableCollision)
{
	simulatePBDWithConstraints(dt, shapes, nullptr, numSubsteps, numPosIters, bEnableCollision);
}
