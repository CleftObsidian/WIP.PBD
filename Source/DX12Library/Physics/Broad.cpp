#include "Broad.h"

void GetBroadCollisionPairs(std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>& shapes, std::vector<BroadCollisionPair>& out)
{
	BroadCollisionPair pair;

	std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>::iterator shape;
	std::unordered_map<size_t, std::shared_ptr<DX12Library::RigidBodyShape>>::iterator otherShape;
	for (shape = shapes.begin(); shape != shapes.end(); ++shape)
	{
		std::shared_ptr<DX12Library::RigidBodyShape> s1 = shape->second;
		otherShape = shape;
		++otherShape;
		for (; otherShape != shapes.end(); ++otherShape)
		{
			std::shared_ptr<DX12Library::RigidBodyShape> s2 = otherShape->second;

			float shapeDistanceSq = XMVectorGetX(XMVector3LengthSq(s1->worldPosition - s2->worldPosition));
			float maxDistanceForCollision = s1->boundingSphereRadius + s2->boundingSphereRadius + 0.1f;
			if (shapeDistanceSq <= maxDistanceForCollision * maxDistanceForCollision)
			{
				pair.s1_id = shape->first;
				pair.s2_id = otherShape->first;
				out.push_back(pair);
			}
		}
	}
}