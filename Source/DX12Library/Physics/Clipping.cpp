#include "Clipping.h"
#include "Support.h"

void GetClippingContactManifold(Collider* collider1, Collider* collider2, XMVECTOR normal, float penetration, std::vector<ColliderContact>& contacts)
{
	if (collider1->type == ColliderType::SPHERE)
	{
		XMVECTOR sphereCollisionPoint = supportPoint(collider1, normal);

		ColliderContact contact;
		contact.collision_point1 = sphereCollisionPoint;
		contact.collision_point2 = sphereCollisionPoint - penetration * normal;
		contact.collision_normal = normal;

		contacts.push_back(contact);
	}
	else if (collider2->type == ColliderType::SPHERE)
	{
		XMVECTOR inverseNormal = -normal;
		XMVECTOR sphereCollisionPoint = supportPoint(collider2, inverseNormal);

		ColliderContact contact;
		contact.collision_point1 = sphereCollisionPoint + penetration * normal;
		contact.collision_point2 = sphereCollisionPoint;
		contact.collision_normal = normal;

		contacts.push_back(contact);
	}
	else
	{
		assert(collider1->type == ColliderType::CONVEX_HULL);
		assert(collider2->type == ColliderType::CONVEX_HULL);

		// TODO: convex-convex-contact-manifold
	}
}