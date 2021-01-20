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
		virtual bool IsCollision(Geometric* const Rhs ,
								Vector3& PushDir,
								float& CrossAreaScale)&abstract;
		virtual void Update(const Vector3 Scale,  
							const Vector3 Rotation , 
							const Vector3 Location)& abstract;
		virtual Type GetType() const& abstract;
	};

	class DLL_DECL AABB : public Geometric
	{
	public:
		AABB(const Vector3 LocalMin, const Vector3 LocalMax);
		virtual Type GetType() const& override;
		virtual void Update(const Vector3 Scale,
							const Vector3 Rotation,
							const Vector3 Location)&override;
		virtual bool IsCollision(Geometric* const Rhs,
								Vector3& PushDir,
								float& CrossAreaScale) & override;
		bool IsCollisionAABB(Geometric* const Rhs ,
							Vector3& PushDir, 
							float& CrossAreaScale)const&;
		Vector3 Min;
		Vector3 Max;
		const Vector3 LocalMax;
		const Vector3 LocalMin;
	};
}