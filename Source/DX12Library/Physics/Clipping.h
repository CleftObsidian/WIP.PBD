#pragma once

#include "Collider.h"

void GetClippingContactManifold(Collider* collider1, Collider* collider2, XMVECTOR normal, float penetration, std::vector<ColliderContact>& contacts);