#include "Geometric.h"

Engine::AABB::AABB(const Vector3 Min, const Vector3 Max)
	: Min{ Min }, Max{ Max }
{
}

Engine::Geometric::Type Engine::AABB::GetType() const&
{
    return Engine::Geometric::Type::AABB;
}

bool Engine::AABB::IsCollision(Geometric* const Rhs)&
{
		switch (Rhs->GetType())
		{
		case Type::AABB:
			return IsCollisionAABB(Rhs);
			break;
		default:
			break;
		}

		return false;
}

bool Engine::AABB::IsCollisionAABB(
	Geometric* const Rhs) const&
{
	auto RhsAABB = static_cast<AABB* const>(Rhs);

	if (RhsAABB->Min.x > Max.x || RhsAABB->Max.x < Min.x)
		return false;
	if (RhsAABB->Min.y > Max.y || RhsAABB->Max.y < Min.y)
		return false;
	if (RhsAABB->Min.z > Max.z || RhsAABB->Max.z < Min.z)
		return false;

	return true;
}
