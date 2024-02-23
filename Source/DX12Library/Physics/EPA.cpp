#include "EPA.h"
#include "Support.h"

void polytopeFromGJKSimplex(const GJKSimplex* s, std::vector<XMVECTOR>& polytope, std::vector<XMINT3>& faces)
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

void getFaceNormalAndDistanceToOrigin(XMINT3 face, std::vector<XMVECTOR>& polytope, XMVECTOR* _normal, float* _distance)
{
	XMVECTOR a = polytope[face.x];
	XMVECTOR b = polytope[face.y];
	XMVECTOR c = polytope[face.z];

	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR normal = XMVector3Normalize(XMVector3Cross(ab, ac));

	assert(XMVectorGetX(normal) != 0.0f || XMVectorGetY(normal) != 0.0f || XMVectorGetZ(normal) != 0.0f);

	// When this value is not 0, it is possible that the normals are not found even if the polytope is not degenerate
	constexpr float DISTANCE_TO_ORIGIN_TOLERANCE = 0.0000000000000;

	// distance from the face's plane to the origin (considering an infinite plane)
	float distance = XMVectorGetX(XMVector3Dot(normal, a));
	if (distance < -DISTANCE_TO_ORIGIN_TOLERANCE)
	{
		// if the distance is less than 0, it means that our normal is point inwards instead of outwards
		// in this case, we just invert both normal and distance
		// this way, we don't need to worry about face's winding
		normal = -normal;
		distance = -distance;
	}
	else if (-DISTANCE_TO_ORIGIN_TOLERANCE <= distance && distance <= DISTANCE_TO_ORIGIN_TOLERANCE)
	{
		// if the distance is exactly 0.0, then it means that the origin is lying exactly on the face.
		// in this case, we can't directly infer the orientation of the normal.
		// since our shape is convex, we analyze the other vertices of the hull to deduce the orientation
		bool was_able_to_calculate_normal = false;
		for (size_t i = 0; i < polytope.size(); ++i) {
			XMVECTOR current = polytope[i];
			float auxiliarDistance = XMVectorGetX(XMVector3Dot(normal, current));
			if (auxiliarDistance < -DISTANCE_TO_ORIGIN_TOLERANCE || DISTANCE_TO_ORIGIN_TOLERANCE < auxiliarDistance)
			{
				// since the shape is convex, the other vertices should always be "behind" the normal plane
				normal = auxiliarDistance < -DISTANCE_TO_ORIGIN_TOLERANCE ? normal : -normal;
				was_able_to_calculate_normal = true;
				break;
			}
		}

		// If we were not able to calculate the normal, it means that ALL points of the polytope are in the same plane
		// Therefore, we either have a degenerate polytope or our tolerance is not big enough
		assert(true == was_able_to_calculate_normal);
	}

	*_normal = normal;
	*_distance = distance;
}

void addEdge(std::vector<XMINT2>& edges, XMINT2 edge, std::vector<XMVECTOR>& polytope)
{
	for (size_t i = 0; i < edges.size(); ++i)
	{
		XMINT2 current = edges[i];
		if (edge.x == current.x && edge.y == current.y)
		{
			edges.erase(edges.begin() + i);
			return;
		}
		if (edge.x == current.y && edge.y == current.x)
		{
			edges.erase(edges.begin() + i);
			return;
		}

		XMVECTOR current_v1 = polytope[current.x];
		XMVECTOR current_v2 = polytope[current.y];
		XMVECTOR edge_v1 = polytope[edge.x];
		XMVECTOR edge_v2 = polytope[edge.y];

		if (true == XMVector3Equal(current_v1, edge_v1) && true == XMVector3Equal(current_v2, edge_v2))
		{
			edges.erase(edges.begin() + i);
			return;
		}
		if (true == XMVector3Equal(current_v1, edge_v2) && true == XMVector3Equal(current_v2, edge_v1))
		{
			edges.erase(edges.begin() + i);
			return;
		}
	}

	edges.push_back(edge);
}

static XMVECTOR triangleCentroid(XMVECTOR p1, XMVECTOR p2, XMVECTOR p3)
{
	XMVECTOR centroid = p2 + p3 + p1;
	centroid /= 3.0f;

	return centroid;
}

bool EPA(Collider* collider1, Collider* collider2, GJKSimplex* simplex, XMVECTOR* _normal, float* _penetration)
{
	std::vector<XMVECTOR> polytope;
	std::vector<XMINT3> faces;

	// Build initial polytope from GJK simplex
	polytopeFromGJKSimplex(simplex, polytope, faces);

	std::vector<XMVECTOR> normals;
	normals.reserve(128);
	std::vector<float> facesDistanceToOrigin;
	facesDistanceToOrigin.reserve(128);

	XMVECTOR minNormal = XMVectorZero();
	float minDistance = FLT_MAX;

	for (size_t i = 0; i < faces.size(); ++i)
	{
		XMVECTOR normal;
		float distance;
		XMINT3 face = faces[i];

		getFaceNormalAndDistanceToOrigin(face, polytope, &normal, &distance);

		normals.push_back(normal);
		facesDistanceToOrigin.push_back(distance);

		if (distance < minDistance)
		{
			minDistance = distance;
			minNormal = normal;
		}
	}

	std::vector<XMINT2> edges;
	edges.reserve(1024);
	bool bConverged = false;
	for (size_t it = 0; it < 100; ++it)
	{
		XMVECTOR supportPoint = SupportPointOfMinkowskiDifference(collider1, collider2, minNormal);

		// If the support time lies on the face currently set as the closest to the origin, we are done.
		float d = XMVectorGetX(XMVector3Dot(minNormal, supportPoint));
		if (fabsf(d - minDistance) < FLT_EPSILON)
		{
			*_normal = minNormal;
			*_penetration = minDistance;
			bConverged = true;

			break;
		}

		// Add new point to polytope
		size_t newPointIndex = polytope.size();
		polytope.push_back(supportPoint);

		// Expand polytope
		for (size_t i = 0; i < normals.size(); ++i)
		{
			XMVECTOR normal = normals[i];
			XMINT3 face = faces[i];

			// If the face normal points towards the support point, we need to reconstruct it.
			XMVECTOR centroid = triangleCentroid(polytope[face.x], polytope[face.y], polytope[face.z]);
			if (0.0f < XMVectorGetX(XMVector3Dot(normal, supportPoint - centroid)))
			{
				XMINT3 face = faces[i];

				XMINT2 edge1 = { face.x, face.y };
				XMINT2 edge2 = { face.y, face.z };
				XMINT2 edge3 = { face.z, face.x };

				addEdge(edges, edge1, polytope);
				addEdge(edges, edge2, polytope);
				addEdge(edges, edge3, polytope);

				// Relative order between the two arrays should be kept
				faces.erase(faces.begin() + i);
				facesDistanceToOrigin.erase(facesDistanceToOrigin.begin() + i);
				normals.erase(normals.begin() + i);

				--i;
			}
		}

		for (size_t i = 0; i < edges.size(); ++i)
		{
			XMINT2 edge = edges[i];
			XMINT3 newFace = { edge.x, edge.y, static_cast<int32_t>(newPointIndex) };
			faces.push_back(newFace);

			XMVECTOR newFaceNormal;
			float newFaceDistance;
			getFaceNormalAndDistanceToOrigin(newFace, polytope, &newFaceNormal, &newFaceDistance);

			normals.push_back(newFaceNormal);
			facesDistanceToOrigin.push_back(newFaceDistance);
		}

		minDistance = FLT_MAX;
		for (size_t i = 0; i < facesDistanceToOrigin.size(); ++i)
		{
			float distance = facesDistanceToOrigin[i];
			if (distance < minDistance)
			{
				minDistance = distance;
				minNormal = normals[i];
			}
		}

		edges.clear();
	}

	if (false == bConverged)
	{
		OutputDebugString(L"EPA didn't converge.\n");
	}

	return bConverged;
}