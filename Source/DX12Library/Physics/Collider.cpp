#include "Collider.h"
#include "Clipping.h"
#include <unordered_map>

static bool doTrianglesShareSameVertex(XMINT3 t1, XMINT3 t2)
{
	return t1.x == t2.x || t1.x == t2.y || t1.x == t2.z ||
		t1.y == t2.x || t1.y == t2.y || t1.y == t2.z ||
		t1.z == t2.x || t1.z == t2.y || t1.z == t2.z;
}

static bool doFacesShareSameVertex(std::vector<WORD>& s1, std::vector<WORD>& s2)
{
	for (size_t i = 0; i < s1.size(); ++i)
	{
		WORD i1 = s1[i];
		for (size_t j = 0; j < s2.size(); ++j)
		{
			WORD i2 = s2[j];
			if (i1 == i2)
			{
				return true;
			}
		}
	}

	return false;
}

static bool isNeighborAlreadyInVertexToNeighborsMap(std::vector<WORD>& vertexToNeighbors, WORD neighbor)
{
	for (size_t i = 0; i < vertexToNeighbors.size(); ++i)
	{
		if (vertexToNeighbors[i] == neighbor)
		{
			return true;
		}
	}

	return false;
}

static void collectFacesPlanarTo(std::vector<XMFLOAT3>* hull, std::vector<XMINT3>& hullTriangleFaces, std::vector<WORD>* triangleFacesToNeighborFacesMap,
	std::vector<bool>& abIsTriangleFaceAlreadyProcessed, WORD faceToTestIndex, XMVECTOR tangentNormal, std::vector<XMINT3>* out)
{
	XMINT3 faceToTest = hullTriangleFaces[faceToTestIndex];
	XMFLOAT3 v1 = hull->at(faceToTest.x);
	XMFLOAT3 v2 = hull->at(faceToTest.y);
	XMFLOAT3 v3 = hull->at(faceToTest.z);

	XMVECTOR v12 = XMLoadFloat3(&v2) - XMLoadFloat3(&v1);
	XMVECTOR v13 = XMLoadFloat3(&v3) - XMLoadFloat3(&v1);
	XMVECTOR faceNormal = XMVector3Normalize(XMVector3Cross(v12, v13));

	if (true == abIsTriangleFaceAlreadyProcessed[faceToTestIndex])
	{
		return;
	}

	float projection = XMVectorGetX(XMVector3Dot(faceNormal, tangentNormal));

	if ((projection - 1.0f) > -FLT_EPSILON && (projection - 1.0f) < FLT_EPSILON)
	{
		out->push_back(faceToTest);
		abIsTriangleFaceAlreadyProcessed[faceToTestIndex] = true;

		std::vector<WORD> neighborFaces = triangleFacesToNeighborFacesMap[faceToTestIndex];

		for (size_t i = 0; i < neighborFaces.size(); ++i)
		{
			WORD neighborFaceIndex = neighborFaces[i];
			collectFacesPlanarTo(hull, hullTriangleFaces, triangleFacesToNeighborFacesMap, abIsTriangleFaceAlreadyProcessed, neighborFaceIndex, tangentNormal, out);
		}
	}
}

static int32_t getEdgeIndex(const std::vector<XMINT2> edges, XMINT2 edge)
{
	for (size_t i = 0; i < edges.size(); ++i)
	{
		XMINT2 currentEdge = edges[i];
		if (currentEdge.x == edge.x && currentEdge.y == edge.y)
		{
			return i;
		}
		if (currentEdge.x == edge.y && currentEdge.y == edge.x)
		{
			return i;
		}
	}

	return -1;
}

static ColliderConvexHullFace createConvexHullFace(std::vector<XMINT3> triangles, XMVECTOR faceNormal)
{
	std::vector<XMINT2> edges;

	// Collect the edges that form the border of the face
	for (size_t i = 0; i < triangles.size(); ++i)
	{
		XMINT3 triangle = triangles[i];

		XMINT2 edge1 = { triangle.x, triangle.y };
		XMINT2 edge2 = { triangle.y, triangle.z };
		XMINT2 edge3 = { triangle.z, triangle.x };

		int32_t edge1Index = getEdgeIndex(edges, edge1);
		if (0 <= edge1Index)
		{
			edges.erase(edges.begin() + edge1Index);
		}
		else
		{
			edges.push_back(edge1);
		}

		int32_t edge2Index = getEdgeIndex(edges, edge2);
		if (0 <= edge2Index)
		{
			edges.erase(edges.begin() + edge2Index);
		}
		else
		{
			edges.push_back(edge2);
		}

		int32_t edge3Index = getEdgeIndex(edges, edge3);
		if (0 <= edge3Index)
		{
			edges.erase(edges.begin() + edge3Index);
		}
		else
		{
			edges.push_back(edge3);
		}
	}

	// Nicely order the edges
	for (size_t i = 0; i < edges.size(); ++i)
	{
		XMINT2 currentEdge = edges[i];
		for (size_t j = i + 1; j < edges.size(); ++j)
		{
			XMINT2 candidateEdge = edges[j];

			if (currentEdge.y != candidateEdge.x && currentEdge.y != candidateEdge.y)
			{
				continue;
			}

			if (currentEdge.y == candidateEdge.y)
			{
				int32_t temp = candidateEdge.x;
				candidateEdge.x = candidateEdge.y;
				candidateEdge.y = temp;
			}

			XMINT2 temp = edges[i + 1];
			edges[i + 1] = candidateEdge;
			edges[j] = temp;
		}
	}

	assert(edges[0].x == edges[edges.size() - 1].y);

	// Simply create the face elements based on the edges
	std::vector<WORD> faceElements;
	for (size_t i = 0; i < edges.size(); ++i)
	{
		XMINT2 currentEdge = edges[i];
		faceElements.push_back(currentEdge.x);
	}

	ColliderConvexHullFace face;
	face.elements = faceElements;
	face.normal = faceNormal;

	return face;
}

Collider CreateColliderConvexHull(const std::vector<XMFLOAT3> vertices, const std::vector<WORD> indices)
{
	std::unordered_map<XMFLOAT3, WORD> vertexToIndexMap;

	// Build hull, eliminating duplicated vertex
	std::vector<XMFLOAT3>* hull = new std::vector<XMFLOAT3>;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		XMFLOAT3 currentVertex = vertices[i];
		WORD currentIndex;
		if (vertexToIndexMap.find(currentVertex) == vertexToIndexMap.end())
		{
			currentIndex = hull->size();
			hull->push_back(currentVertex);
			vertexToIndexMap.emplace(currentVertex, currentIndex);
		}
	}

	// Collect all triangle faces that compose the just-built hull
	std::vector<XMINT3> hullTriangleFaces;
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		WORD i1 = indices[i];
		WORD i2 = indices[i + 1];
		WORD i3 = indices[i + 2];
		XMFLOAT3 v1 = vertices[i1];
		XMFLOAT3 v2 = vertices[i2];
		XMFLOAT3 v3 = vertices[i3];

		WORD new_i1 = vertexToIndexMap[v1];
		WORD new_i2 = vertexToIndexMap[v2];
		WORD new_i3 = vertexToIndexMap[v3];

		XMINT3 triangle = { new_i1, new_i2, new_i3 };
		hullTriangleFaces.push_back(triangle);
	}

	// Prepare vertex to faces map
	std::vector<WORD>* vertexToFacesMap = new std::vector<WORD>[hull->size()];

	// Prepare vertex to neighbors map
	std::vector<WORD>* vertexToNeighborsMap = new std::vector<WORD>[hull->size()];

	// Prepare triangle faces to neighbors map
	std::vector<WORD>* triangleFacesToNeighborFacesMap = new std::vector<WORD>[hullTriangleFaces.size()];

	// Create the vertex to neighbors map
	for (size_t i = 0; i < hullTriangleFaces.size(); ++i)
	{
		XMINT3 triangleFace = hullTriangleFaces[i];

		for (size_t j = 0; j < hullTriangleFaces.size(); ++j)
		{
			if (i == j)
			{
				continue;
			}

			XMINT3 faceToTest = hullTriangleFaces[j];
			if (true == doTrianglesShareSameVertex(triangleFace, faceToTest))
			{
				triangleFacesToNeighborFacesMap[i].push_back(j);
			}
		}

		// Fill vertex to edges map
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.x], triangleFace.y))
		{
			vertexToNeighborsMap[triangleFace.x].push_back(triangleFace.y);
		}
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.x], triangleFace.z))
		{
			vertexToNeighborsMap[triangleFace.x].push_back(triangleFace.z);
		}
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.y], triangleFace.x))
		{
			vertexToNeighborsMap[triangleFace.y].push_back(triangleFace.x);
		}
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.y], triangleFace.z))
		{
			vertexToNeighborsMap[triangleFace.y].push_back(triangleFace.z);
		}
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.z], triangleFace.x))
		{
			vertexToNeighborsMap[triangleFace.z].push_back(triangleFace.x);
		}
		if (false == isNeighborAlreadyInVertexToNeighborsMap(vertexToNeighborsMap[triangleFace.z], triangleFace.y))
		{
			vertexToNeighborsMap[triangleFace.z].push_back(triangleFace.y);
		}
	}

	// Collect all 'de facto' faces of the convex hull
	std::vector<ColliderConvexHullFace>* faces = new std::vector<ColliderConvexHullFace>;
	std::vector<bool> abIsTriangleFaceAlreadyProcessed;
	abIsTriangleFaceAlreadyProcessed.reserve(hullTriangleFaces.size());

	for (size_t i = 0; i < hullTriangleFaces.size(); ++i)
	{
		if (true == abIsTriangleFaceAlreadyProcessed[i])
		{
			continue;
		}

		XMINT3 triangleFace = hullTriangleFaces[i];
		XMFLOAT3 v1 = hull->at(triangleFace.x);
		XMFLOAT3 v2 = hull->at(triangleFace.y);
		XMFLOAT3 v3 = hull->at(triangleFace.z);

		XMVECTOR v12 = XMLoadFloat3(&v2) - XMLoadFloat3(&v1);
		XMVECTOR v13 = XMLoadFloat3(&v3) - XMLoadFloat3(&v1);
		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(v12, v13));

		std::vector<XMINT3> planarFaces;
		collectFacesPlanarTo(hull, hullTriangleFaces, triangleFacesToNeighborFacesMap, abIsTriangleFaceAlreadyProcessed, i, normal, &planarFaces);

		ColliderConvexHullFace newFace = createConvexHullFace(planarFaces, normal);
		WORD newFaceIndex = faces->size();
		faces->push_back(newFace);

		// Fill vertex to faces map accordingly
		for (size_t i = 0; i < planarFaces.size(); ++i)
		{
			XMINT3 planarFace = planarFaces[i];
			vertexToFacesMap[planarFace.x].push_back(newFaceIndex);
			vertexToFacesMap[planarFace.y].push_back(newFaceIndex);
			vertexToFacesMap[planarFace.z].push_back(newFaceIndex);
		}
	}

	// Prepare face to neighbors map
	std::vector<WORD>* faceToNeighborFacesMap = new std::vector<WORD>[faces->size()];

	// Fill faces to neighbor faces map
	for (size_t i = 0; i < faces->size(); ++i)
	{
		ColliderConvexHullFace face = faces->at(i);

		for (size_t j = 0; j < faces->size(); ++j)
		{
			if (i == j)
			{
				continue;
			}

			ColliderConvexHullFace candidateFace = faces->at(j);
			if (doFacesShareSameVertex(face.elements, candidateFace.elements))
			{
				faceToNeighborFacesMap[i].push_back(j);
			}
		}
	}

	delete[] triangleFacesToNeighborFacesMap;

	ColliderConvexHull convexHull;
	convexHull.vertices = hull;
	convexHull.transformedVertices = hull;
	convexHull.faces = faces;
	convexHull.transformedFaces = faces;
	convexHull.vertexToFaces = vertexToFacesMap;
	convexHull.vertexToNeighbors = vertexToNeighborsMap;
	convexHull.faceToNeighbors = faceToNeighborFacesMap;

	Collider collider;
	collider.type = ColliderType::CONVEX_HULL;
	collider.convexHull = convexHull;

	return collider;
}

Collider CreateColliderSphere(const float radius)
{
	Collider collider;
	collider.type = ColliderType::SPHERE;
	collider.sphere.center = XMVectorZero();
	collider.sphere.radius = radius;

	return collider;
}

static void updateCollider(Collider* collider, XMVECTOR translation, const XMVECTOR rotationQ)
{
	UNREFERENCED_PARAMETER(rotationQ);

	switch (collider->type)
	{
	case ColliderType::CONVEX_HULL:
		XMMATRIX modelMatrixNoScale = XMMatrixRotationQuaternion(rotationQ) * XMMatrixTranslationFromVector(translation);
		for (size_t i = 0; i < collider->convexHull.transformedVertices->size(); ++i)
		{
			XMVECTOR vertex = XMLoadFloat3(&collider->convexHull.vertices->at(i));
			XMVECTOR transformedVertex = XMVector3Transform(vertex, modelMatrixNoScale);
			XMStoreFloat3(&collider->convexHull.transformedVertices->at(i), transformedVertex);
		}

		for (size_t i = 0; i < collider->convexHull.transformedFaces->size(); ++i)
		{
			XMVECTOR normal = collider->convexHull.faces->at(i).normal;
			XMVECTOR transformedNormal = XMVector3Transform(normal, modelMatrixNoScale);
			collider->convexHull.transformedFaces->at(i).normal = XMVector3Normalize(transformedNormal);
		}
		break;
	case ColliderType::SPHERE:
		collider->sphere.center = translation;
		break;
	default:
		assert(false);
		break;
	}
}

void UpdateColliders(std::vector<Collider>& colliders, XMVECTOR translation, const XMVECTOR rotationQ)
{
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		Collider* collider = &colliders[i];
		updateCollider(collider, translation, rotationQ);
	}
}

static void destroyColliderConvexHull(Collider* collider)
{
	delete collider->convexHull.vertices;
	delete collider->convexHull.transformedFaces;
	delete collider->convexHull.faces;
	delete collider->convexHull.transformedFaces;
	delete[] collider->convexHull.vertexToFaces;
	delete[] collider->convexHull.vertexToNeighbors;
	delete[] collider->convexHull.faceToNeighbors;
}

static void destroyCollider(Collider* collider)
{
	switch (collider->type)
	{
	case ColliderType::CONVEX_HULL:
		destroyColliderConvexHull(collider);
		break;
	case ColliderType::SPHERE:
		break;
	default:
		break;
	}
}

void DestroyColliders(std::vector<Collider>& colliders)
{
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		Collider* collider = &colliders[i];
		destroyCollider(collider);
	}
}

XMMATRIX GetCollidersDefaultInertiaTensor(std::vector<Collider>& colliders, float mass)
{
	if (1 == colliders.size())
	{
		Collider* collider = &colliders[0];

		if (collider->type == ColliderType::SPHERE)
		{
			assert(0.0f == XMVectorGetX(XMVector3LengthSq(collider->sphere.center)));

			float inertia = 2.0f / 5.0f * mass * collider->sphere.radius * collider->sphere.radius;
			XMMATRIX result = XMMatrixScaling(inertia, inertia, inertia);
			result.r[3] = XMVectorZero();

			return result;
		}
	}

	size_t totalNumVertices = 0;
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		Collider* collider = &colliders[i];
		totalNumVertices += collider->convexHull.vertices->size();
	}

	float massPerVertex = mass / static_cast<float>(totalNumVertices);

	XMMATRIX result;
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		Collider* collider = &colliders[i];
		assert(collider->type == ColliderType::CONVEX_HULL);

		for (size_t j = 0; j < collider->convexHull.vertices->size(); ++j)
		{
			XMFLOAT3 v = collider->convexHull.vertices->at(j);
			result.r[0] = XMVectorSet((v.y * v.y + v.z * v.z), v.x * v.y, v.x * v.z, 0.0f);
			result.r[1] = XMVectorSet(v.x * v.y, (v.x * v.x + v.z * v.z), v.y * v.z, 0.0f);
			result.r[2] = XMVectorSet(v.x * v.z, v.y * v.z, (v.x * v.x + v.y * v.y), 0.0f);
			result.r[3] = XMVectorZero();
			result *= massPerVertex;
		}
	}

	return result;
}

static float getConvexHullColliderBoundingSphereRadius(const Collider* collider)
{
	float maxDistance = 0.0f;
	for (size_t i = 0; i < collider->convexHull.vertices->size(); ++i)
	{
		XMFLOAT3 v = collider->convexHull.vertices->at(i);
		float distance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&v)));
		if (maxDistance < distance)
		{
			maxDistance = distance;
		}
	}

	return maxDistance;
}

static float getColliderBoundingSphereRadius(const Collider* collider)
{
	switch (collider->type)
	{
	case ColliderType::CONVEX_HULL:
		return getConvexHullColliderBoundingSphereRadius(collider);
		break;
	case ColliderType::SPHERE:
		return collider->sphere.radius;
		break;
	default:
		assert(false);
		break;
	}

	assert(false);
	return -1.0f;
}

float GetCollidersBoundingSphereRadius(const std::vector<Collider>& colliders)
{
	float maxBoundingSphereRadius = -FLT_MAX;
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		const Collider* collider = &colliders[i];
		float boundingSphereRadius = getColliderBoundingSphereRadius(collider);
		if (maxBoundingSphereRadius < boundingSphereRadius)
		{
			maxBoundingSphereRadius = boundingSphereRadius;
		}
	}

	return maxBoundingSphereRadius;
}

static void getColliderContacts(Collider* collider1, Collider* collider2, std::vector<ColliderContact>& contacts)
{
	float penetration;
	XMVECTOR normal;

	if (collider1->type == ColliderType::SPHERE && collider2->type == ColliderType::SPHERE)
	{
		XMVECTOR distanceVector = collider2->sphere.center - collider1->sphere.center;
		float distanceSquared = XMVectorGetX(XMVector3Dot(distanceVector, distanceVector));
		float minDistance = collider2->sphere.radius + collider1->sphere.radius;
		if (distanceSquared < (minDistance * minDistance))
		{
			normal = XMVector3Normalize(distanceVector);
			penetration = minDistance - sqrtf(distanceSquared);
			GetClippingContactManifold(collider1, collider2, normal, penetration, contacts);
		}
	}

	// TODO: need to GJK-EPA algorithm implementation for convex hull collision
}

std::vector<ColliderContact> GetCollidersContacts(std::vector<Collider>& colliders1, std::vector<Collider>& colliders2)
{
	std::vector<ColliderContact> contacts;
	contacts.reserve(16);

	for (size_t i = 0; i < colliders1.size(); ++i)
	{
		Collider* collider1 = &colliders1[i];
		for (size_t j = 0; j < colliders2.size(); ++j)
		{
			Collider* collider2 = &colliders2[j];
			getColliderContacts(collider1, collider2, contacts);
		}
	}

	return contacts;
}