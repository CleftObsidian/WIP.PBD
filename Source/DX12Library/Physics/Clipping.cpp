#include "Clipping.h"
#include "Support.h"

struct Plane
{
	XMVECTOR normal;
	XMVECTOR point;
};

static bool isPointInPlane(const Plane* plane, XMVECTOR position)
{
	float distance = -XMVectorGetX(XMVector3Dot(plane->normal, plane->point));
	if (XMVectorGetX(XMVector3Dot(position, plane->normal)) + distance < 0.0f)
	{
		return false;
	}

	return true;
}

static bool planeEdgeIntersection(const Plane* plane, const XMVECTOR start, const XMVECTOR end, XMVECTOR* outPoint)
{
	const float EPSILON = 0.000001f;
	XMVECTOR ab = end - start;

	// Check that the edge and plane are not parallel and thus never intersect
	// We do this by projecting the line (start - A, End - B) ab along the plane
	float ab_p = XMVectorGetX(XMVector3Dot(plane->normal, ab));
	if (EPSILON < fabsf(ab_p))
	{
		// Generate a random point on the plane
		float distance = -XMVectorGetX(XMVector3Dot(plane->normal, plane->point));
		XMVECTOR p_co = -distance * plane->normal;

		// Work out the edge factor to scale edge by
		// e.g. how far along the edge to traverse before it meets the plane.
		// This is computed by: -proj<plane_nrml>(edge_start - any_planar_point) / proj<plane_nrml>(edge_start - edge_end)
		float fac = -XMVectorGetX(XMVector3Dot(plane->normal, start - p_co)) / ab_p;

		// Stop any large floating point divide issues with almost parallel planes
		fac = fminf(fmaxf(fac, 0.0f), 1.0f);

		// Return point on edge
		*outPoint = start + fac * ab;

		return true;
	}

	return false;
}

// Clips the input polygon to the input clip planes
// If remove_instead_of_clipping is true, vertices that are lying outside the clipping planes will be removed instead of clipped
// Based on https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics5collisionmanifolds/
static void sutherland_hodgman(std::vector<XMVECTOR>* inputPolygon, size_t numClipPlanes, const Plane* clipPlanes,
	std::vector<XMVECTOR>* outPolygon, bool removeInsteadOfClipping)
{
	assert(nullptr != outPolygon);
	assert(0 < numClipPlanes);

	// Create temporary list of vertices
	// We will keep ping-pong'ing between the two lists updating them as we go.
	std::vector<XMVECTOR>* input = new std::vector<XMVECTOR>;
	for (size_t i = 0; i < inputPolygon->size(); ++i)
	{
		input->push_back(inputPolygon->at(i));
	}
	std::vector<XMVECTOR>* output = new std::vector<XMVECTOR>;
	output->reserve(inputPolygon->size() * 4);

	for (size_t i = 0; i < numClipPlanes; ++i)
	{
		// If every single point has already been removed previously, just exit
		if (true == input->empty())
		{
			break;
		}

		const Plane* plane = &clipPlanes[i];

		// Loop through each edge of the polygon and clip that edge against the current plane.
		XMVECTOR startPoint = *(input->end() - 1);
		XMVECTOR tempPoint = startPoint;
		for (size_t j = 0; j < input->size(); ++j)
		{
			XMVECTOR endPoint = input->at(j);
			bool bStartInPlane = isPointInPlane(plane, startPoint);
			bool bEndInPlane = isPointInPlane(plane, endPoint);

			if (true == removeInsteadOfClipping)
			{
				if (true == bEndInPlane)
				{
					output->push_back(endPoint);
				}
			}
			// If the edge is entirely within the clipping plane, keep it as it is
			else if (true == bStartInPlane && true == bEndInPlane)
			{
				output->push_back(endPoint);
			}
			// If the edge intersects the clipping plane, cut the edge along clip plane
			else if (true == bStartInPlane && false == bEndInPlane)
			{
				if (true == planeEdgeIntersection(plane, startPoint, endPoint, &tempPoint))
				{
					output->push_back(tempPoint);
				}
			}
			else if (false == bStartInPlane && true == bEndInPlane)
			{
				if (true == planeEdgeIntersection(plane, startPoint, endPoint, &tempPoint))
				{
					output->push_back(tempPoint);
				}

				output->push_back(endPoint);
			}
			// ..otherwise the edge is entirely outside the clipping plane and should be removed/ignored

			startPoint = endPoint;
		}

		// Swap input/output polygons, and clear output list for us to generate afresh
		std::vector<XMVECTOR>* temp = input;
		input = output;
		output = temp;
		output->clear();
	}

	outPolygon = input;
	delete output;
}

static XMVECTOR getClosestPointPolygon(XMVECTOR position, Plane* referencePlane)
{
	float d = XMVectorGetX(XMVector3Dot(-referencePlane->normal, referencePlane->point));
	return position - (XMVectorGetX(XMVector3Dot(referencePlane->normal, position)) + d) * referencePlane->normal;
}

static std::vector<Plane>* buildBoundaryPlanes(ColliderConvexHull* convexHull, size_t targetFaceIndex)
{
	std::vector<Plane>* result = new std::vector<Plane>;
	result->reserve(16);
	std::vector<WORD>* faceNeighbors = &convexHull->faceToNeighbors[targetFaceIndex];

	for (size_t i = 0; i < faceNeighbors->size(); ++i)
	{
		ColliderConvexHullFace neighborFace = convexHull->transformedFaces->at(faceNeighbors->at(i));
		Plane p;
		p.point = convexHull->transformedVertices->at(neighborFace.elements[0]);
		p.normal = -neighborFace.normal;
		result->push_back(p);
	}

	return result;
}

static size_t getFaceWithMostFittingNormal(size_t supportIndex, const ColliderConvexHull* convexHull, XMVECTOR normal)
{
	std::vector<WORD> supportFaces = convexHull->vertexToFaces[supportIndex];

	float maxProj = -FLT_MAX;
	size_t selectedFaceIndex = SIZE_MAX;
	for (size_t i = 0; i < supportFaces.size(); ++i)
	{
		ColliderConvexHullFace face = convexHull->transformedFaces->at(supportFaces[i]);
		float proj = XMVectorGetX(XMVector3Dot(face.normal, normal));
		if (maxProj < proj)
		{
			maxProj = proj;
			selectedFaceIndex = supportFaces[i];
		}
	}

	return selectedFaceIndex;
}

static XMINT4 getEdgeWithMostFittingNormal(size_t support1Index, size_t support2Index, const ColliderConvexHull* convexHull1,
	const ColliderConvexHull* convexHull2, XMVECTOR normal, XMVECTOR* edgeNormal)
{
	XMVECTOR invertedNormal = -normal;

	XMVECTOR support1 = convexHull1->transformedVertices->at(support1Index);
	XMVECTOR support2 = convexHull2->transformedVertices->at(support2Index);

	std::vector<WORD>* support1Neighbors = &convexHull1->vertexToNeighbors[support1Index];
	std::vector<WORD>* support2Neighbors = &convexHull2->vertexToNeighbors[support2Index];

	float maxDot = -FLT_MAX;
	XMINT4 selectedEdges = XMINT4();
	for (size_t i = 0; i < support1Neighbors->size(); ++i)
	{
		XMVECTOR neighbor1 = convexHull1->transformedVertices->at(support1Neighbors->at(i));
		XMVECTOR edge1 = support1 - neighbor1;
		for (size_t j = 0; j < support2Neighbors->size(); ++j)
		{
			XMVECTOR neighbor2 = convexHull2->transformedVertices->at(support2Neighbors->at(j));
			XMVECTOR edge2 = support2 - neighbor2;

			XMVECTOR currentNormal = XMVector3Normalize(XMVector3Cross(edge1, edge2));
			XMVECTOR currentNormalInverted = -currentNormal;

			float dot = XMVectorGetX(XMVector3Dot(currentNormal, normal));
			if (maxDot < dot)
			{
				maxDot = dot;
				selectedEdges.x = static_cast<int32_t>(support1Index);
				selectedEdges.y = support1Neighbors->at(i);
				selectedEdges.z = static_cast<int32_t>(support2Index);
				selectedEdges.w = support2Neighbors->at(j);
				*edgeNormal = currentNormal;
			}

			dot = XMVectorGetX(XMVector3Dot(currentNormalInverted, normal));
			if (maxDot < dot)
			{
				maxDot = dot;
				selectedEdges.x = static_cast<int32_t>(support1Index);
				selectedEdges.y = support1Neighbors->at(i);
				selectedEdges.z = static_cast<int32_t>(support2Index);
				selectedEdges.w = support2Neighbors->at(j);
				*edgeNormal = currentNormalInverted;
			}
		}
	}

	return selectedEdges;
}

// This function calculates the distance between two independent skew lines in the 3D world
// The first line is given by a known point P1 and a direction vector D1
// The second line is given by a known point P2 and a direction vector D2
// Outputs:
// L1 is the closest POINT to the second line that belongs to the first line
// L2 is the closest POINT to the first line that belongs to the second line
// _N is the number that satisfies L1 = P1 + _N * D1
// _M is the number that satisfies L2 = P2 + _M * D2
static bool collisionDistanceBetweenSkewLines(XMVECTOR p1, XMVECTOR d1, XMVECTOR p2, XMVECTOR d2, XMVECTOR* l1, XMVECTOR* l2, float* _n, float* _m)
{
	float n1 = XMVectorGetX(d1) * XMVectorGetX(d2) + XMVectorGetY(d1) * XMVectorGetY(d2) + XMVectorGetZ(d1) * XMVectorGetZ(d2);
	float n2 = XMVectorGetX(d2) * XMVectorGetX(d2) + XMVectorGetY(d2) * XMVectorGetY(d2) + XMVectorGetZ(d2) * XMVectorGetZ(d2);
	float m1 = -XMVectorGetX(d1) * XMVectorGetX(d1) - XMVectorGetY(d1) * XMVectorGetY(d1) - XMVectorGetZ(d1) * XMVectorGetZ(d1);
	float m2 = -XMVectorGetX(d2) * XMVectorGetX(d1) - XMVectorGetY(d2) * XMVectorGetY(d1) - XMVectorGetZ(d2) * XMVectorGetZ(d1);
	float r1 = -XMVectorGetX(d1) * XMVectorGetX(p2) + XMVectorGetX(d1) * XMVectorGetX(p1) - XMVectorGetY(d1) * XMVectorGetY(p2)
		+ XMVectorGetY(d1) * XMVectorGetY(p1) - XMVectorGetZ(d1) * XMVectorGetZ(p2) + XMVectorGetZ(d1) * XMVectorGetZ(p1);
	float r2 = -XMVectorGetX(d2) * XMVectorGetX(p2) + XMVectorGetX(d2) * XMVectorGetX(p1) - XMVectorGetY(d2) * XMVectorGetY(p2)
		+ XMVectorGetY(d2) * XMVectorGetY(p1) - XMVectorGetZ(d2) * XMVectorGetZ(p2) + XMVectorGetZ(d2) * XMVectorGetZ(p1);


	// Solve 2x2 linear system
	if (0 == (n1 * m2) - (n2 * m1))
	{
		return false;
	}
	float n = ((r1 * m2) - (r2 * m1)) / ((n1 * m2) - (n2 * m1));
	float m = ((n1 * r2) - (n2 * r1)) / ((n1 * m2) - (n2 * m1));

	if (nullptr != l1)
	{
		*l1 = p1 + m * d1;
	}
	if (nullptr != l2)
	{
		*l2 = p2 + n * d2;
	}
	if (nullptr != _n)
	{
		*_n = n;
	}
	if (nullptr != _m)
	{
		*_m = m;
	}

	return true;
}

static std::vector<XMVECTOR>* getVerticesOfFaces(ColliderConvexHull* hull, ColliderConvexHullFace face)
{
	std::vector<XMVECTOR>* vertices = new std::vector<XMVECTOR>;
	vertices->reserve(16);
	for (size_t i = 0; i < face.elements.size(); ++i)
	{
		vertices->push_back(hull->transformedVertices->at(face.elements[i]));
	}

	return vertices;
}

void convexToConvexContactManifold(Collider* collider1, Collider* collider2, XMVECTOR normal, std::vector<ColliderContact>& contacts)
{
	assert(collider1->type == ColliderType::CONVEX_HULL);
	assert(collider2->type == ColliderType::CONVEX_HULL);

	ColliderConvexHull* convexHull1 = &collider1->convexHull;
	ColliderConvexHull* convexHull2 = &collider2->convexHull;

	constexpr float EPSILON = 0.0001f;

	XMVECTOR invertedNormal = -normal;
	
	XMVECTOR edgeNormal;
	size_t support1Index = GetSupportPointIndex(convexHull1, normal);
	size_t support2Index = GetSupportPointIndex(convexHull2, invertedNormal);
	size_t face1Index = getFaceWithMostFittingNormal(support1Index, convexHull1, normal);
	size_t face2Index = getFaceWithMostFittingNormal(support2Index, convexHull2, invertedNormal);
	ColliderConvexHullFace face1 = convexHull1->transformedFaces->at(face1Index);
	ColliderConvexHullFace face2 = convexHull2->transformedFaces->at(face2Index);
	XMINT4 edges = getEdgeWithMostFittingNormal(support1Index, support2Index, convexHull1, convexHull2, normal, &edgeNormal);

	float chosenNormal1Dot = XMVectorGetX(XMVector3Dot(face1.normal, normal));
	float chosenNormal2Dot = XMVectorGetX(XMVector3Dot(face2.normal, invertedNormal));
	float edgeNormalDot = XMVectorGetX(XMVector3Dot(edgeNormal, normal));

	if (chosenNormal1Dot + EPSILON < edgeNormalDot && chosenNormal2Dot + EPSILON < edgeNormalDot)
	{
		// Edge
		XMVECTOR l1 = XMVectorZero();
		XMVECTOR l2 = XMVectorZero();
		XMVECTOR p1 = convexHull1->transformedVertices->at(edges.x);
		XMVECTOR d1 = convexHull1->transformedVertices->at(edges.y) - p1;
		XMVECTOR p2 = convexHull2->transformedVertices->at(edges.z);
		XMVECTOR d2 = convexHull2->transformedVertices->at(edges.w) - p2;
		assert(true == collisionDistanceBetweenSkewLines(p1, d1, p2, d2, &l1, &l2, 0, 0));

		ColliderContact contact = { l1, l2, normal };
		contacts.push_back(contact);
	}
	else
	{
		// Face
		bool bIsFace1ReferenceFace = chosenNormal1Dot > chosenNormal2Dot;
		std::vector<XMVECTOR>* referenceFaceSupportPoints = bIsFace1ReferenceFace ? getVerticesOfFaces(convexHull1, face1) : getVerticesOfFaces(convexHull2, face2);
		std::vector<XMVECTOR>* incidentFaceSupportPoints = bIsFace1ReferenceFace ? getVerticesOfFaces(convexHull2, face2) : getVerticesOfFaces(convexHull1, face1);

		std::vector<Plane>* boundaryPlanes = bIsFace1ReferenceFace ? buildBoundaryPlanes(convexHull1, face1Index) : buildBoundaryPlanes(convexHull2, face2Index);

		std::vector<XMVECTOR>* clippedPoints = nullptr;
		sutherland_hodgman(incidentFaceSupportPoints, boundaryPlanes->size(), boundaryPlanes->data(), clippedPoints, false);

		Plane referencePlane;
		referencePlane.normal = bIsFace1ReferenceFace ? -face1.normal : -face2.normal;
		referencePlane.point = referenceFaceSupportPoints->at(0);

		std::vector<XMVECTOR>* finalClippedPoints = nullptr;
		sutherland_hodgman(clippedPoints, 1, &referencePlane, finalClippedPoints, true);

		for (size_t i = 0; i < finalClippedPoints->size(); ++i)
		{
			XMVECTOR point = finalClippedPoints->at(i);
			XMVECTOR closestPoint = getClosestPointPolygon(point, &referencePlane);
			XMVECTOR pointDifference = point - closestPoint;
			float contactPenetration = XMVectorGetX(XMVector3Dot(pointDifference, normal));

			// we are projecting the points that are in the incident face on the reference planes
			// so the points that we have are part of the incident object.
			ColliderContact contact;
			if (true == bIsFace1ReferenceFace)
			{
				contact.collision_point1 = point - contactPenetration * normal;
				contact.collision_point2 = point;
			}
			else
			{
				contact.collision_point1 = point;
				contact.collision_point2 = point + contactPenetration * normal;
			}
			contact.collision_normal = normal;

			if (contactPenetration < 0.0f)
			{
				contacts.push_back(contact);
			}
		}

		delete referenceFaceSupportPoints;
		delete incidentFaceSupportPoints;
		delete boundaryPlanes;
		delete clippedPoints;
		delete finalClippedPoints;
	}

	if (true == contacts.empty())
	{
		OutputDebugString(L"Warning: no intersection was found\n");
	}
}

void GetClippingContactManifold(Collider* collider1, Collider* collider2, XMVECTOR normal, float penetration, std::vector<ColliderContact>& contacts)
{
	if (collider1->type == ColliderType::SPHERE)
	{
		XMVECTOR sphereCollisionPoint = SupportPoint(collider1, normal);

		ColliderContact contact;
		contact.collision_point1 = sphereCollisionPoint;
		contact.collision_point2 = sphereCollisionPoint - penetration * normal;
		contact.collision_normal = normal;

		contacts.push_back(contact);
	}
	else if (collider2->type == ColliderType::SPHERE)
	{
		XMVECTOR inverseNormal = -normal;
		XMVECTOR sphereCollisionPoint = SupportPoint(collider2, inverseNormal);

		ColliderContact contact;
		contact.collision_point1 = sphereCollisionPoint + penetration * normal;
		contact.collision_point2 = sphereCollisionPoint;
		contact.collision_normal = normal;

		contacts.push_back(contact);
	}
	else
	{
		convexToConvexContactManifold(collider1, collider2, normal, contacts);
	}
}