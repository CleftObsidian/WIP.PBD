#pragma once

#include "Common.h"

struct ColliderContact
{
	XMVECTOR collision_point1;
	XMVECTOR collision_point2;
	XMVECTOR collision_normal;
};

struct ColliderConvexHullFace
{
	std::vector<WORD> elements;
	XMVECTOR normal;
};

struct ColliderConvexHull
{
	std::vector<XMFLOAT3>* vertices;
	std::vector<XMFLOAT3>* transformedVertices;
	std::vector<ColliderConvexHullFace>* faces;
	std::vector<ColliderConvexHullFace>* transformedFaces;

	std::vector<WORD>* vertexToFaces;
	std::vector<WORD>* vertexToNeighbors;
	std::vector<WORD>* faceToNeighbors;
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
		ColliderConvexHull convexHull;
		ColliderSphere sphere;
	};
};

Collider CreateColliderConvexHull(const std::vector<XMFLOAT3> vertices, const std::vector<WORD> indices);
Collider CreateColliderSphere(const float radius);

void UpdateColliders(std::vector<Collider>& colliders, XMVECTOR translation, const XMVECTOR rotationQ);
void DestroyColliders(std::vector<Collider>& colliders);
XMMATRIX GetCollidersDefaultInertiaTensor(std::vector<Collider>& colliders, float mass);
float GetCollidersBoundingSphereRadius(const std::vector<Collider>& colliders);
std::vector<ColliderContact> GetCollidersContacts(std::vector<Collider>& colliders1, std::vector<Collider>& colliders2);