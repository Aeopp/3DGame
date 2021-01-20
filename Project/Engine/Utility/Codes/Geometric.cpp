#include "Geometric.h"
#include "FMath.hpp"
#include "imgui.h"
#include <iostream>

Engine::AABB::AABB(const Vector3 LocalMin, const Vector3 LocalMax)
					: LocalMin{ LocalMin }, LocalMax{ LocalMax }
{
}

Engine::Geometric::Type Engine::AABB::GetType() const&
{
    return Engine::Geometric::Type::AABB;
}

void Engine::AABB::Update(const Vector3 Scale,
							const Vector3 Rotation,
							const Vector3 Location)&
{
	const Matrix ToWorld = FMath::WorldMatrix(Scale, { 0,0,0 }, Location);
	Min = FMath::Mul(LocalMin, ToWorld);
	Max = FMath::Mul(LocalMax, ToWorld);
}

bool Engine::AABB::IsCollision
	(Geometric* const Rhs, 
	Vector3& PushDir, float& CrossAreaScale)&
{
	switch (Rhs->GetType())
	{
	case Type::AABB:
		return IsCollisionAABB(Rhs ,PushDir,CrossAreaScale);
		break;
	default:
		break;
	}

	return false;
}

bool Engine::AABB::IsCollisionAABB(
	Geometric* const Rhs,
	Vector3& PushDir,
	float& CrossAreaScale) const&
{
	auto RhsAABB = static_cast<AABB* const>(Rhs);

	if (RhsAABB->Min.x > Max.x || RhsAABB->Max.x < Min.x)
		return false;
	if (RhsAABB->Min.y > Max.y || RhsAABB->Max.y < Min.y)
		return false;
	if (RhsAABB->Min.z > Max.z || RhsAABB->Max.z < Min.z)
		return false;
	//const std::array<const float, 3u> CrossAxis
	//{
	//	std::fabsf(Max.x - RhsAABB->Min.x),
	//	std::fabsf(Max.y - Min.y),
	//	std::fabsf(Max.z - Min.z),
	//};
	//
	//std::min_element

	return true;
}
