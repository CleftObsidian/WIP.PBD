#pragma once

#include "Common.h"
#include "Collider.h"
#include "Shapes/RigidBodyShape.h"

struct PositionalConstraintPreprocessedData 
{
	std::shared_ptr<DX12Library::RigidBodyShape> s1;
	std::shared_ptr<DX12Library::RigidBodyShape> s2;
	XMVECTOR r1_world;
	XMVECTOR r2_world;
	XMMATRIX s1_inverseInertiaTensor;
	XMMATRIX s2_inverseInertiaTensor;
};

// Positional constraint
// TODO