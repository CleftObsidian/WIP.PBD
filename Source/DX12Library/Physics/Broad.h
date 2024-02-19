#pragma once

#include "PBD.h"

struct BroadCollisionPair
{
	size_t s1_id;
	size_t s2_id;
};

void GetBroadCollisionPairs(std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes, std::vector<BroadCollisionPair>& out);