#include "Support.h"

XMVECTOR supportPoint(Collider* collider, XMVECTOR direction)
{
	switch (collider->type)
	{
	case ColliderType::SPHERE:
		return (collider->sphere.center + collider->sphere.radius * direction);
		break;
	case ColliderType::CONVEX_HULL:
		break;
	}

	assert(false);
	return XMVectorZero();
}

XMVECTOR supportPointOfMinkowskiDifference(Collider* collider1, Collider* collider2, XMVECTOR direction)
{
	XMVECTOR support1 = supportPoint(collider1, direction);
	XMVECTOR support2 = supportPoint(collider2, direction);

	return support1 - support2;
}