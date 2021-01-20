#include "CollisionSystem.h"
#include "Collision.h"


void Engine::CollisionSystem::Regist(
	CollisionTag _Tag, 
	Collision* const _Collision)& 
{

	CollisionCompMap[_Tag].push_back(_Collision);
}

void Engine::CollisionSystem::Initialize()&
{

}

void Engine::CollisionSystem::Update(const float DeltaTime)&
{
	for (auto& [Tag, CollisionComps] : CollisionCompMap)
	{
		for (auto& Lhs : CollisionComps)
		{
			for (auto& CurrentCheckTag : Lhs->RefCollisionables())
			{
				for (auto& Rhs : CollisionCompMap[CurrentCheckTag])
				{
					if (Lhs == Rhs)continue;

					Lhs->IsCollision(Rhs);
				}
			}
		}
	}

	CollisionCompMap.clear();
}
