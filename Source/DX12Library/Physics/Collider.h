#pragma once

#include "Common.h"

struct ColliderContact
{
	XMVECTOR collision_point1;
	XMVECTOR collision_point2;
	XMVECTOR collision_normal;
};

struct ColliderSphere
{
	XMVECTOR center;
	float radius;
};

enum class ColliderType
{
	SPHERE,
	CONVEX_HULL
};

struct Collider
{
	ColliderType type;
	union
	{
		ColliderSphere sphere;
	};
};

Collider CreateColliderSphere(const float radius);
void UpdateColliders(std::vector<Collider>& colliders, XMVECTOR translation, XMVECTOR rotationQ);
XMMATRIX GetCollidersDefaultInertiaTensor(std::vector<Collider>& colliders, float mass);
float GetCollidersBoundingSphereRadius(const std::vector<Collider>& colliders);
std::vector<ColliderContact> GetCollidersContacts(std::vector<Collider>& colliders1, std::vector<Collider>& colliders2);