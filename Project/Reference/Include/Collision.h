#pragma once
#include "Component.h"
#include "Geometric.h"
#include <set>
#include "CollisionSystem.h"

namespace Engine
{
	class DLL_DECL Collision : public Component
	{
	public:
		using Super = Component;
		void Initialize(const CollisionTag _Tag)&;
		virtual void Update(class Object* const Owner,
			const float DeltaTime) & override;
		virtual void Event(class Object* Owner) & override;
		auto& RefCollisionables()&;
		bool IsCollisionable(const CollisionTag _Tag)const&;
		bool IsCollision(Collision* const Rhs)&;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Collision;
		CollisionTag _Tag{ CollisionTag::None };
		std::unique_ptr<Geometric> _Geometric;
	private:
		std::set<CollisionTag> Collisionables;
	};
};

inline auto& Engine::Collision::RefCollisionables()&
{
	return Collisionables;
};

inline bool Engine::Collision::IsCollisionable(const CollisionTag _Tag) const&
{
	return Collisionables.contains(_Tag);
}
