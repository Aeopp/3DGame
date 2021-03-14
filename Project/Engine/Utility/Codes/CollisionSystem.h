#pragma once
#include "TypeAlias.h"
#include "SingletonInterface.h"
#include "DllHelper.H"
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace Engine
{
	enum class CollisionTag :uint8
	{
		None,
		NPC,
		Decorator,
		Player,
		PlayerAttack,
		Enemy,
		EnemyAttack,
	};

	class CollisionSystem : public SingletonInterface<CollisionSystem >
	{
	public:
		void Regist( CollisionTag _Tag,
			class Collision* const _Collision)&;
		void UnRegist(CollisionTag _Tag,
			class Collision* const _Collision)&;
		void Initialize()&;
		void Update(const float DeltaTime)&;
	private:
		std::set<uint32> DeleteCollisionIDs{};
		std::unordered_map<uint32, class Collision*> CollisionCompIDMap{};
		std::map<CollisionTag, std::vector<class Collision*> >
			CollisionCompMap{};
	};
}




