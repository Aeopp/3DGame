#include "CollisionSystem.h"
#include "Collision.h"
#include "Object.h"
#include "imgui.h"

void Engine::CollisionSystem::Regist(
	CollisionTag _Tag,
	Collision* const _Collision)&
{
	CollisionCompMap[_Tag].push_back(_Collision);
	CollisionCompIDMap[_Collision->ID] = _Collision;
};

void Engine::CollisionSystem::Initialize()&
{

};

void Engine::CollisionSystem::Update(const float DeltaTime)&
{
#ifdef DEVELOP
	ImGui::Begin("Collision Information");
#endif

	for (auto& [Tag, CollisionComps] : CollisionCompMap)
	{
		for (auto& Lhs : CollisionComps)
		{
			if (!Lhs->bCollision)continue;

			for (auto& CurrentCheckTag : Lhs->RefCollisionables())
			{
				for(auto& Rhs : CollisionCompMap[CurrentCheckTag])
				{
					if (Lhs->ID == Rhs->ID || !Rhs->bCollision)continue;

					Lhs->IsCollision(Rhs);
				}
			}
			Lhs->bImmobility = true;
		}
	}

	for (auto& [Tag, CollisionComps] : CollisionCompMap)
	{
		for (auto& Lhs : CollisionComps)
		{
			if (!Lhs->bCollision)continue;

			for (auto iter = std::begin(Lhs->HitCollisionIDs); iter != std::end(Lhs->HitCollisionIDs); )
			{
				const uint32 HitID = *iter;
				if (!Lhs->CurrentCheckedCollisionIDs.contains(HitID))
				{
					Lhs->Owner->HitEnd(CollisionCompIDMap[HitID]->Owner);
					Lhs->HitCollisionIDs.erase(iter++);
				}
				else ++iter;
			}
		}
	}

#ifdef DEVELOP
	ImGui::End();
#endif

	CollisionCompMap.clear();
}
