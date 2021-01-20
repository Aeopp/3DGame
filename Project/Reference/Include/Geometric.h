#pragma once
#include "DllHelper.H"
#include <type_traits>
#include "TypeAlias.h"
#include <typeinfo>

namespace Engine
{
	class DLL_DECL Geometric
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

	class DLL_DECL AABB : public Geometric
	{
	public:
		AABB(const Vector3 Min, const Vector3 Max);
		virtual Type GetType() const& override;
		virtual bool IsCollision(Geometric* const Rhs) & override;
		bool IsCollisionAABB(Geometric* const Rhs)const&;
		Vector3 Min;
		Vector3 Max;
	};
}