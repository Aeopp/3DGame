#include "CollisionSystem.h"
#include "Collision.h"
#include "Object.h"
#include "imgui.h"
#include "UtilityGlobal.h"

void Engine::CollisionSystem::Regist(
	CollisionTag _Tag,
	Collision* const _Collision)&
{
	CollisionCompMap[_Tag].push_back(_Collision);
	CollisionCompIDMap[_Collision->ID] = _Collision;
}
void Engine::CollisionSystem::UnRegist(CollisionTag _Tag, Collision* const _Collision)&
{
	// 충돌 시스템에서 정보를 삭제.
	CollisionCompIDMap.erase(_Collision->ID);
	DeleteCollisionIDs.erase(_Collision->ID);
};

void Engine::CollisionSystem::Initialize()&
{

};

void Engine::CollisionSystem::Update(const float DeltaTime)&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Collision Information");
	}

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
					if (!DeleteCollisionIDs.contains(HitID))
					{
						if (auto RhsIter = CollisionCompIDMap.find(HitID); 
							RhsIter != std::end(CollisionCompIDMap))
						{
							Lhs->Owner->HitEnd(RhsIter->second->Owner);
						}
					}
					Lhs->HitCollisionIDs.erase(iter++);
				}
				else ++iter;
			}
		}
	}

	if (Engine::Global::bDebugMode)
	{
		ImGui::End();
	}
	

	CollisionCompMap.clear();
}
