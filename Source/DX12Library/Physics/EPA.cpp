#include "EPA.h"

static void polytopeFromGJKSimplex(const GJKSimplex* s, std::vector<XMVECTOR>& polytope, std::vector<XMINT3>& faces)
{
	assert(s->num == 4);
	polytope.reserve(4);
	faces.reserve(4);

	polytope.push_back(s->a);
	polytope.push_back(s->b);
	polytope.push_back(s->c);
	polytope.push_back(s->d);

	XMINT3 i1 = { 0, 1, 2 };	// ABC
	XMINT3 i2 = { 0, 2, 3 };	// ACD
	XMINT3 i3 = { 0, 3, 1 };	// ADB
	XMINT3 i4 = { 1, 2, 3 };	// BCD

	faces.push_back(i1);
	faces.push_back(i2);
	faces.push_back(i3);
	faces.push_back(i4);
}

bool EPA(Collider* collider1, Collider* collider2, GJKSimplex* simplex, XMVECTOR* normal, float* penetration)
{
	std::vector<XMVECTOR> polytope;
	std::vector<XMINT3> faces;

	// Build initial polytope from GJK simplex
	polytopeFromGJKSimplex(simplex, polytope, faces);

	std::vector<XMVECTOR> normals;
	normals.reserve(128);
	std::vector<float> facesDistanceToOrigin;
	facesDistanceToOrigin.reserve(128);

	XMVECTOR minNormal;
	float minDistance = FLT_MAX;

	// TODO
}