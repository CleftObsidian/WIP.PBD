#pragma once

#include "Collider.h"

XMVECTOR supportPoint(Collider* collider, XMVECTOR direction);
XMVECTOR supportPointOfMinkowskiDifference(Collider* collider1, Collider* collider2, XMVECTOR direction);