#include "Collider.h"
#include "Clipping.h"

Collider CreateColliderSphere(const float radius)
{
	Collider collider;
	collider.type = ColliderType::SPHERE;
	collider.sphere.center = XMVectorZero();
	collider.sphere.radius = radius;

	return collider;
}

static void updateCollider(Collider* collider, XMVECTOR translation, XMVECTOR rotationQ)
{
	UNREFERENCED_PARAMETER(rotationQ);

	switch (collider->type)
	{
	case ColliderType::SPHERE:
		collider->sphere.center = translation;
		break;
	case ColliderType::CONVEX_HULL:
		break;
	default:
		assert(false);
		break;
	}
}

void UpdateColliders(std::vector<Collider>& colliders, XMVECTOR translation, XMVECTOR rotationQ)
{
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		Collider* collider = &colliders[i];
		updateCollider(collider, translation, rotationQ);
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
			XMMATRIX I = XMMatrixScaling(inertia, inertia, inertia);
			I.r[3] = XMVectorZero();

			return I;
		}
	}
	
	assert(false);
	
	return XMMatrixIdentity();
}

static float getColliderBoundingSphereRadius(const Collider* collider)
{
	switch (collider->type)
	{
	case ColliderType::SPHERE:
		return collider->sphere.radius;
		break;
	case ColliderType::CONVEX_HULL:
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