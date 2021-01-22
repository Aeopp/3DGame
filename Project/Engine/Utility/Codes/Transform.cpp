#include "Camera.h"
#include "Transform.h"
#include "FMath.hpp"
#include "imgui.h"

void Engine::Transform::Initialize()&
{
	Super::Initialize();

	UpdateWorld();
};

void Engine::Transform::Update(Object* const Owner, const float DeltaTime)&
{

};

void Engine::Transform::Event(Object* const Owner)&
{
	Super::Event(Owner);

	const Vector3& Scale = GetScale();
	const Vector3& Location = GetLocation();
	const Vector3& Rotation = GetRotation();

	ImGui::Text(
		"Scale\n X : %f \n Y : %f \n Z : %f", Scale.x, Scale.y, Scale.z);
	ImGui::Text(
		"Location\n X : %f \n Y : %f \n Z : %f", Location.x, Location.y, Location.z);
	ImGui::Text(
		"Rotation\n X : %f \n Y : %f \n Z : %f",Rotation.x, Rotation.y, Rotation.z);
};

const Matrix& Engine::Transform::UpdateWorld()&
{
	World = FMath::WorldMatrix(Scale, Forward, Right, Up, Location);
	return World;
};

void Engine::Transform::RotateYaw(const float Radian, const float DeltaTime)&
{
	RotateAxis(GetUp(), Radian, DeltaTime);
}

void Engine::Transform::RotateRoll(const float Radian, const float DeltaTime)&
{
	RotateAxis(GetForward(), Radian, DeltaTime);
}

void Engine::Transform::RotatePitch(const float Radian, const float DeltaTime)&
{
	RotateAxis( GetRight(), Radian, DeltaTime);
}

void Engine::Transform::RotateAxis(Vector3 Axis, const float Radian,
	const float DeltaTime)&
{
	Axis = FMath::Normalize(Axis);
	Forward = FMath::Normalize(FMath::RotationVecNormal(Forward, Axis, Radian * DeltaTime));
	Right= FMath::Normalize(FMath::RotationVecNormal(Right, Axis, Radian * DeltaTime ));
	Up = FMath::Normalize(FMath::RotationVecNormal(Up , Axis, Radian * DeltaTime));

	Vector3 _Unit = Forward;
	_Unit.x = 0.f;
	_Unit = FMath::Normalize(_Unit);
	Rotation.x = std::acosf(FMath::Dot(_Unit, { 0,0,1 }));

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

void Engine::Transform::MoveForward(const float DeltaTime, const float Speed)
{
	Move(Forward, DeltaTime, Speed);
}

void Engine::Transform::MoveRight(const float DeltaTime, const float Speed)
{
	Move(Right, DeltaTime, Speed);
}

void Engine::Transform::MoveUp(const float DeltaTime, const float Speed)
{
	Move(Up, DeltaTime, Speed);
};


