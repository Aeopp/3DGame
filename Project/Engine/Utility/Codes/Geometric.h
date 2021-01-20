#pragma once
#include <type_traits>
#include "TypeAlias.h"
#include <typeinfo>

namespace Engine
{
	class Geometric
	{
	protected:
		enum class Type : uint8
		{
			None,
			AABB,
		};
	public:
		virtual bool IsCollision(Geometric* const Rhs)&abstract;
		virtual Type GetType() const& abstract;
	};

	class AABB : public Geometric
	{
	public:
		virtual Type GetType() const& override;
		virtual bool IsCollision(Geometric* const Rhs) & override;
		bool IsCollisionAABB(Geometric* const Rhs)const&;
		Vector3 Min;
		Vector3 Max;
	};
}