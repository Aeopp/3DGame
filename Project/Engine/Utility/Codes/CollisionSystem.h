#pragma once
#include "SingletonInterface.h"
#include "DllHelper.H"
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace Engine
{
	enum class CollisionTag :uint8
	{
		None,
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

		void Initialize()&;
		void Update(const float DeltaTime)&;
	private:
		std::map<CollisionTag, std::vector<class Collision*> >
			CollisionCompMap{};
	};
}




