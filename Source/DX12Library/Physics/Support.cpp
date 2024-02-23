#include "Support.h"

size_t GetSupportPointIndex(ColliderConvexHull* convexHull, XMVECTOR direction)
{
	size_t selectedIndex = SIZE_MAX;
	float maxDot = -FLT_MAX;
	for (size_t i = 0; i < convexHull->transformedVertices->size(); ++i)
	{
		float dot = XMVectorGetX(XMVector3Dot(convexHull->transformedVertices->at(i), direction));
		if (maxDot < dot)
		{
			selectedIndex = i;
			maxDot = dot;
		}
	}

	return selectedIndex;
}

XMVECTOR SupportPoint(Collider* collider, XMVECTOR direction)
{
	switch (collider->type)
	{
	case ColliderType::SPHERE:
		return (collider->sphere.center + collider->sphere.radius * direction);
		break;
	case ColliderType::CONVEX_HULL:
		size_t selectedIndex = GetSupportPointIndex(&collider->convexHull, direction);
		return collider->convexHull.transformedVertices->at(selectedIndex);
		break;
	}

	assert(false);
	return XMVectorZero();
}

XMVECTOR SupportPointOfMinkowskiDifference(Collider* collider1, Collider* collider2, XMVECTOR direction)
{
	XMVECTOR support1 = SupportPoint(collider1, direction);
	XMVECTOR support2 = SupportPoint(collider2, direction);

	return support1 - support2;
}