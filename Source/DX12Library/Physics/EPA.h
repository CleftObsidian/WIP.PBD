#pragma once

#include "Common.h"
#include "GJK.h"

bool EPA(Collider* collider1, Collider* collider2, GJKSimplex* simplex, XMVECTOR* normal, float* penetration);