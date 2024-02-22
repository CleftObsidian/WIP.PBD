#include "PBDBaseConstraint.h"

void CalculatePositionalConstraintPreprocessedData(std::shared_ptr<DX12Library::RigidBodyShape> s1, std::shared_ptr<DX12Library::RigidBodyShape> s2, XMVECTOR r1_local, XMVECTOR r2_local, PositionalConstraintPreprocessedData* pcpd)
{
	pcpd->s1 = s1;
	pcpd->s2 = s2;

	pcpd->r1_world = XMVector3Rotate(r1_local, s1->worldRotation);
	pcpd->r2_world = XMVector3Rotate(r2_local, s2->worldRotation);

	pcpd->s1_inverseInertiaTensor = s1->GetDynamicInverseInertiaTensor();
	pcpd->s2_inverseInertiaTensor = s2->GetDynamicInverseInertiaTensor();
}

float GetPositionalConstraintDeltaLambda(PositionalConstraintPreprocessedData* pcpd, float h, float compliance, float lambda, XMVECTOR delta_x)
{
	float c = XMVectorGetX(XMVector3Length(delta_x));

	if (c <= FLT_EPSILON)
	{
		return 0.0f;
	}

	std::shared_ptr<DX12Library::RigidBodyShape> s1 = pcpd->s1;
	std::shared_ptr<DX12Library::RigidBodyShape> s2 = pcpd->s2;
	XMVECTOR r1_world = pcpd->r1_world;
	XMVECTOR r2_world = pcpd->r2_world;
	XMMATRIX s1_inverseInertiaTensor = pcpd->s1_inverseInertiaTensor;
	XMMATRIX s2_inverseInertiaTensor = pcpd->s2_inverseInertiaTensor;

	XMVECTOR n = delta_x / c;

	// Calculate the inverse masses of both shapes
	float w1 = s1->inverseMass + XMVectorGetX(XMVector3Dot(XMVector3Cross(r1_world, n), XMVector3Transform(XMVector3Cross(r1_world, n), s1_inverseInertiaTensor)));
	float w2 = s2->inverseMass + XMVectorGetX(XMVector3Dot(XMVector3Cross(r2_world, n), XMVector3Transform(XMVector3Cross(r2_world, n), s2_inverseInertiaTensor)));

	assert(0.0f != w1 + w2);

	// Calculate the delta_lambda (XPBD) and updates the constraint
	float compliance_til = compliance / (h * h);
	float delta_lambda = (-c - compliance_til * lambda) / (w1 + w2 + compliance_til);

	return delta_lambda;
}

void ApplyPositionalConstraint(PositionalConstraintPreprocessedData* pcpd, float delta_lambda, XMVECTOR delta_x)
{
	float c = XMVectorGetX(XMVector3Length(delta_x));

	if (c <= FLT_EPSILON)
	{
		return;
	}

	std::shared_ptr<DX12Library::RigidBodyShape> s1 = pcpd->s1;
	std::shared_ptr<DX12Library::RigidBodyShape> s2 = pcpd->s2;
	XMVECTOR r1_world = pcpd->r1_world;
	XMVECTOR r2_world = pcpd->r2_world;
	XMMATRIX s1_inverseInertiaTensor = pcpd->s1_inverseInertiaTensor;
	XMMATRIX s2_inverseInertiaTensor = pcpd->s2_inverseInertiaTensor;

	XMVECTOR n = delta_x / c;

	// Calculate the positional impulse
	XMVECTOR positionalImpulse = delta_lambda * n;

	// Update the position of the shapes
	if (false == s1->bFixed)
	{
		s1->worldPosition += s1->inverseMass * positionalImpulse;
	}
	if (false == s2->bFixed)
	{
		s2->worldPosition -= s2->inverseMass * positionalImpulse;
	}

	// Update the rotation of the shapes
	XMVECTOR angular1 = XMVector3Transform(XMVector3Cross(r1_world, positionalImpulse), s1_inverseInertiaTensor);
	XMVECTOR angular2 = XMVector3Transform(XMVector3Cross(r2_world, positionalImpulse), s2_inverseInertiaTensor);
	angular1 = XMVectorSetW(angular1, 0.0f);
	angular2 = XMVectorSetW(angular2, 0.0f);
	XMVECTOR q1 = XMQuaternionMultiply(angular1, s1->worldRotation);
	XMVECTOR q2 = XMQuaternionMultiply(angular2, s2->worldRotation);
	if (false == s1->bFixed)
	{
		s1->worldRotation += 0.5f * q1;
		s1->worldRotation = XMQuaternionNormalize(s1->worldRotation);
	}
	if (false == s2->bFixed)
	{
		s2->worldRotation += 0.5f * q2;
		s2->worldRotation = XMQuaternionNormalize(s2->worldRotation);
	}
}
