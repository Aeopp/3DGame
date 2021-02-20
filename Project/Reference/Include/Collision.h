#pragma once
#include "Component.h"
#include "Geometric.h"
#include <set>
#include "CollisionSystem.h"
#include "RenderInterface.h"

namespace Engine
{
	class DLL_DECL Collision : public Component, public RenderInterface
	{
	public:
		using Super = Component;
		void Initialize(
			IDirect3DDevice9* const Device,
			const CollisionTag _Tag,
			class Transform* const OwnerTransform ,
			const std::string& ClassIdentifier)&;
		virtual void Update(class Object* const Owner,
			const float DeltaTime) & override;
		virtual void Event(class Object* Owner) & override;
		virtual void Render(const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation) & override;
		auto& RefCollisionables()&;
		auto& RefPushCollisionables()&;
		bool IsCollisionable(const CollisionTag _Tag)const&;
		bool IsCollision(Collision* const Rhs)&;

		void Save()&;
		void Load()&;
	public:
		bool bCurrentFrameCollision = false;
		static const inline Property TypeProperty = Property::Collision;
		CollisionTag _Tag{ CollisionTag::None };
		std::unique_ptr<Geometric> _Geometric;
		bool bImmobility = false;
		std::set<uint32> CurrentCheckedCollisionIDs{};
		std::set<uint32> HitCollisionIDs{};
		bool bCollision = true;
		class Object* Owner{ nullptr };
		std::string OwnerClassIdentifier{};
		Matrix OffsetMatrix = FMath::Identity();
	private:
		struct OffsetInformation
		{
			Vector3 Scale{ 1,1,1 };
			Vector3 Rotation{ 0,0,0 };
			Vector3 Location{ 0,0,0 };
		};
		OffsetInformation _OffsetInfo{}; 

		IDirect3DDevice9* Device{ nullptr };  
		class Transform* OwnerTransform{ nullptr };
		std::set<CollisionTag> PushCollisionables;
		std::set<CollisionTag> Collisionables;
	};
};

inline auto& Engine::Collision::RefCollisionables()&
{
	return Collisionables;
}
inline auto& Engine::Collision::RefPushCollisionables()&
{
	return PushCollisionables;
};

inline bool Engine::Collision::IsCollisionable(const CollisionTag _Tag) const&
{
	return Collisionables.contains(_Tag);
};
