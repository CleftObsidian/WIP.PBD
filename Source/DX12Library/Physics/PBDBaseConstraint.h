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
void CalculatePositionalConstraintPreprcessedData(std::shared_ptr<DX12Library::RigidBodyShape> s1, std::shared_ptr<DX12Library::RigidBodyShape> s2,
	XMVECTOR r1_local, XMVECTOR r2_local, PositionalConstraintPreprocessedData* pcpd);
float GetPositionalConstraintDeltaLambda(PositionalConstraintPreprocessedData* pcpd, float h, float compliance, float lambda, XMVECTOR delta_x);
void ApplyPositionalConstraint(PositionalConstraintPreprocessedData* pcpd, float delta_lambda, XMVECTOR delta_x);