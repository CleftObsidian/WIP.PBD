#pragma once

#include "Common.h"
#include "Collider.h"

struct GJKSimplex
{
	XMVECTOR a;
	XMVECTOR b;
	XMVECTOR c;
	XMVECTOR d;
	uint32_t num;
};

bool GJKCollides(Collider* collider1, Collider* collider2, GJKSimplex* simplex);