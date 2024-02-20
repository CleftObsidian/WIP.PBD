#pragma once

#include "Shapes/RigidBodyShape.h"
#include <unordered_map>

enum class PBDAxisType
{
	PBD_POSITIVE_X_AXIS,
	PBD_NEGATIVE_X_AXIS,
	PBD_POSITIVE_Y_AXIS,
	PBD_NEGATIVE_Y_AXIS,
	PBD_POSITIVE_Z_AXIS,
	PBD_NEGATIVE_Z_AXIS
};

enum class ConstraintType
{
	POSITIONAL_CONSTRAINT,
	COLLISION_CONSTRAINT,
};

struct PositionalConstraint
{
	XMVECTOR r1_local;
	XMVECTOR r2_local;
	XMVECTOR distance;
	float compliance;
	float lambda;
};

struct CollisionConstraint
{
	XMVECTOR r1_local;
	XMVECTOR r2_local;
	XMVECTOR normal;
	float lambda_t;
	float lambda_n;
};

struct Constraint
{
	ConstraintType type;
	size_t s1_id;
	size_t s2_id;

	union
	{
		PositionalConstraint positional_constraint;
		CollisionConstraint collision_constraint;
	};
};

void SimulatePBD(float dt, std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes, size_t numSubsteps, size_t numPosIters, bool bEnableCollision);