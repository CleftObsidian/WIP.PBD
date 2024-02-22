#include "PBD.h"
#include "Broad.h"
#include "PBDBaseConstraint.h"

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

	PositionalConstraintPreprocessedData pcpd;
	CalculatePositionalConstraintPreprocessedData(s1, s2, constraint->positional_constraint.r1_local, constraint->positional_constraint.r2_local, &pcpd);
	float delta_lambda = GetPositionalConstraintDeltaLambda(&pcpd, h, constraint->positional_constraint.compliance, constraint->positional_constraint.lambda, delta_x);
	ApplyPositionalConstraint(&pcpd, delta_lambda, delta_x);
	constraint->positional_constraint.lambda += delta_lambda;
}

static void solveCollisionConstraint(Constraint* constraint, float h, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes)
{
	assert(constraint->type == ConstraintType::COLLISION_CONSTRAINT);

	std::shared_ptr<DX12Library::RigidBodyShape> s1 = shapes[constraint->s1_id];
	std::shared_ptr<DX12Library::RigidBodyShape> s2 = shapes[constraint->s2_id];

	PositionalConstraintPreprocessedData pcpd;
	CalculatePositionalConstraintPreprocessedData(s1, s2, constraint->collision_constraint.r1_local, constraint->collision_constraint.r2_local, &pcpd);

	// Calculate p1 and p2 in order to calculate d
	XMVECTOR p1 = s1->worldPosition + pcpd.r1_world;
	XMVECTOR p2 = s2->worldPosition + pcpd.r2_world;
	float d = XMVectorGetX(XMVector3Dot(p1 - p2, constraint->collision_constraint.normal));

	if (0.0f < d)
	{
		XMVECTOR delta_x = d * constraint->collision_constraint.normal;
		float delta_lambda = GetPositionalConstraintDeltaLambda(&pcpd, h, 0.0f, constraint->collision_constraint.lambda_n, delta_x);
		ApplyPositionalConstraint(&pcpd, delta_lambda, delta_x);
		constraint->collision_constraint.lambda_n += delta_lambda;

		// Recalculate shape pair preprocessed data and p1, p2
		CalculatePositionalConstraintPreprocessedData(s1, s2, constraint->collision_constraint.r1_local, constraint->collision_constraint.r2_local, &pcpd);

		p1 = s1->worldPosition + pcpd.r1_world;
		p2 = s2->worldPosition + pcpd.r2_world;

		delta_lambda = GetPositionalConstraintDeltaLambda(&pcpd, h, 0.0f, constraint->collision_constraint.lambda_t, delta_x);

		// Static friction
		const float staticFrictionCoefficient = (s1->staticFrictionCoefficient + s2->staticFrictionCoefficient) * 0.5f;

		float lambda_t = constraint->collision_constraint.lambda_t + delta_lambda;
		float lambda_n = constraint->collision_constraint.lambda_n;
		if (staticFrictionCoefficient * lambda_n < lambda_t)
		{
			XMVECTOR p1_til = s1->prevWorldPosition + XMVector3Rotate(constraint->collision_constraint.r1_local, s1->prevWorldRotation);
			XMVECTOR p2_til = s2->prevWorldPosition + XMVector3Rotate(constraint->collision_constraint.r2_local, s2->prevWorldRotation);
			XMVECTOR delta_p = (p1 - p1_til) - (p2 - p2_til);
			XMVECTOR delta_p_t = delta_p - XMVectorGetX(XMVector3Dot(delta_p, constraint->collision_constraint.normal)) * constraint->collision_constraint.normal;

			ApplyPositionalConstraint(&pcpd, delta_lambda, delta_p_t);
			constraint->collision_constraint.lambda_t += delta_lambda;
		}
	}
}

static void solveConstraint(Constraint* constraint, float h, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes)
{
	switch (constraint->type)
	{
	case ConstraintType::POSITIONAL_CONSTRAINT:
		solvePositionalConstraint(constraint, h, shapes);
		return;
		break;
	case ConstraintType::COLLISION_CONSTRAINT:
		solveCollisionConstraint(constraint, h, shapes);
		return;
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
		}

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

		// PBD velocity update
		//std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>::iterator shape;
		for (shape = shapes.begin(); shape != shapes.end(); ++shape)
		{
			std::shared_ptr<DX12Library::RigidBodyShape> s = shape->second;
			if (true == s->bFixed || false == s->bActive)
			{
				continue;
			}

			// Storing the current velocity for the velocity solver
			s->prevLinearVelocity = s->linearVelocity;
			s->prevAngularVelocity = s->angularVelocity;

			// Update linear velocity based on the position difference
			s->linearVelocity = (1.0f / h) * (s->worldPosition - s->prevWorldPosition);

			// Update angular velocity based on the orientation difference
			XMVECTOR invQ = XMQuaternionInverse(s->prevWorldRotation);
			XMVECTOR delta_q = XMQuaternionMultiply(s->worldRotation, invQ);
			if (0.0f <= XMVectorGetW(delta_q))
			{
				s->angularVelocity = (2.0f / h) * XMVectorSet(XMVectorGetX(delta_q), XMVectorGetY(delta_q), XMVectorGetZ(delta_q), 0.0f);
			}
			else
			{
				s->angularVelocity = (-2.0f / h) * XMVectorSet(XMVectorGetX(delta_q), XMVectorGetY(delta_q), XMVectorGetZ(delta_q), 0.0f);
			}
		}

		// Velocity solver for every collision
		for (size_t j = 0; j < constraints->size(); ++j)
		{
			Constraint* constraint = &constraints->at(j);
			if (constraint->type == ConstraintType::COLLISION_CONSTRAINT)
			{
				std::shared_ptr<DX12Library::RigidBodyShape> s1 = shapes[constraint->s1_id];
				std::shared_ptr<DX12Library::RigidBodyShape> s2 = shapes[constraint->s2_id];
				XMVECTOR n = constraint->collision_constraint.normal;
				float lambda_t = constraint->collision_constraint.lambda_t;
				float lambda_n = constraint->collision_constraint.lambda_n;

				PositionalConstraintPreprocessedData pcpd;
				CalculatePositionalConstraintPreprocessedData(s1, s2, constraint->collision_constraint.r1_local, constraint->collision_constraint.r2_local, &pcpd);

				XMVECTOR v1 = s1->linearVelocity;
				XMVECTOR w1 = s1->angularVelocity;
				XMVECTOR v2 = s2->linearVelocity;
				XMVECTOR w2 = s2->angularVelocity;

				// Calculate the relative normal and tangential velocities at the contact point
				XMVECTOR v = (v1 + XMVector3Cross(w1, pcpd.r1_world)) - (v2 + XMVector3Cross(w2, pcpd.r2_world));
				float vn = XMVectorGetX(XMVector3Dot(n, v));
				XMVECTOR vt = v - vn * n;

				// delta_v stores the velocity change
				XMVECTOR delta_v = XMVectorZero();

				// Coulomb's dynamic friction
				const float dynamicFrictionCoefficient = (s1->dynamicFrictionCoefficient + s2->dynamicFrictionCoefficient) * 0.5f;
				float fn = lambda_n / h;
				float fact = fminf(dynamicFrictionCoefficient * fabsf(fn), XMVectorGetX(XMVector3Length(vt)));
				delta_v += -fact * XMVector3Normalize(vt);

				// Restitution
				XMVECTOR old_v1 = s1->prevLinearVelocity;
				XMVECTOR old_w1 = s1->prevAngularVelocity;
				XMVECTOR old_v2 = s2->prevLinearVelocity;
				XMVECTOR old_w2 = s2->prevAngularVelocity;
				XMVECTOR v_til = (old_v1 - XMVector3Cross(old_w1, pcpd.r1_world)) - (old_v2 - XMVector3Cross(old_w2, pcpd.r2_world));
				float vn_til = XMVectorGetX(XMVector3Dot(n, v_til));
				float e = s1->restitutionCoefficient * s2->restitutionCoefficient;
				fact = -vn + fminf(-e * vn_til, 0.0f);
				delta_v += fact * n;

				// Applying delta_v considering the inverse masses of both shapes
				float _w1 = s1->inverseMass + XMVectorGetX(XMVector3Dot(XMVector3Cross(pcpd.r1_world, n),
					XMVector3Transform(XMVector3Cross(pcpd.r1_world, n), pcpd.s1_inverseInertiaTensor)));
				float _w2 = s2->inverseMass + XMVectorGetX(XMVector3Dot(XMVector3Cross(pcpd.r2_world, n),
					XMVector3Transform(XMVector3Cross(pcpd.r2_world, n), pcpd.s2_inverseInertiaTensor)));
				//float _w1 = s1->inverseMass;
				//float _w2 = s2->inverseMass;
				XMVECTOR p = (1.0f / (_w1 + _w2)) * delta_v;

				if (false == s1->bFixed)
				{
					s1->linearVelocity += s1->inverseMass * p;
					s1->angularVelocity += XMVector3Transform(XMVector3Cross(pcpd.r1_world, p), pcpd.s1_inverseInertiaTensor);
				}
				if (false == s2->bFixed)
				{
					s2->linearVelocity -= s2->inverseMass * p;
					s2->angularVelocity -= XMVector3Transform(XMVector3Cross(pcpd.r2_world, p), pcpd.s2_inverseInertiaTensor);
				}
			}
		}

		delete constraints;
	}
}

void SimulatePBD(float dt, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes, size_t numSubsteps, size_t numPosIters, bool bEnableCollision)
{
	simulatePBDWithConstraints(dt, shapes, nullptr, numSubsteps, numPosIters, bEnableCollision);
}
