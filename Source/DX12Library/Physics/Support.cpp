#include "Support.h"

XMVECTOR support_point(Collider* collider, XMVECTOR direction)
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
