#include "Camera.h"
#include "Transform.h"
#include "FMath.hpp"

void Engine::Transform::Initialize()&
{
	Super::Initialize();

	UpdateWorld();
};

void Engine::Transform::Update(Object* const Owner, const float DeltaTime)&
{

};


void Engine::Transform::UpdateBasis(const Matrix& From)&
{
	Forward = FMath::Normalize({ From._31,From._32,From._33 });
	Right = FMath::Normalize({ From._11,From._12,From._13 });
	Up = FMath::Normalize({ From._21,From._22,From._23 });
};

const Matrix& Engine::Transform::UpdateWorld()&
{
	World = FMath::WorldMatrix(Scale, Rotation, Location);
	UpdateBasis(World);
	return World;
};

void Engine::Transform::Rotate(Vector3 Axis, const float Radian)&
{
	Axis = FMath::Normalize(Axis);

	Matrix RotationMatrix = FMath::Rotation(Rotation)
							* FMath::RotationAxisMatrix(Axis, Radian);

	UpdateBasis(RotationMatrix);

	Vector3 _Unit = Forward;
	_Unit.x = 0.f;
	_Unit = FMath::Normalize(_Unit);
	Rotation.x  = std::acosf(FMath::Dot(_Unit, { 0,0,1 }));

	_Unit = Forward;
	_Unit.y = 0.f;
	_Unit = FMath::Normalize(_Unit);
	Rotation.y = std::acosf(FMath::Dot(_Unit, { 0,0,1 }));

	_Unit = Up;
	_Unit.z = 0.f;
	_Unit = FMath::Normalize(_Unit);
	Rotation.z = std::acosf(FMath::Dot(_Unit, { 0,1,0 }));
}
void Engine::Transform::Move
	(Vector3 Direction, 
	const float DeltaTime, 
	const float Speed)
{
	Direction = FMath::Normalize(Direction);
	Location += Direction * DeltaTime * Speed;
}

