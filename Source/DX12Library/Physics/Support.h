#pragma once

#include "Collider.h"

size_t GetSupportPointIndex(ColliderConvexHull* convexHull, XMVECTOR direction);
XMVECTOR SupportPoint(Collider* collider, XMVECTOR direction);
XMVECTOR SupportPointOfMinkowskiDifference(Collider* collider1, Collider* collider2, XMVECTOR direction);