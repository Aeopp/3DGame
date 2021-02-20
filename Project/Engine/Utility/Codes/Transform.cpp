#include "Camera.h"
#include "Transform.h"
#include "FMath.hpp"
#include "imgui.h"
#include <string>
#include "StringHelper.h"


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

	const Vector3& Scale    = GetScale();
	const Vector3& Location = GetLocation();
	const Vector3& Rotation = GetRotation();

	if (ImGui::TreeNode(("TransformEdit_" + ToA(Owner->GetName())).c_str()))
	{
		ImGui::InputFloat3("Scale", (float*)&Scale);
		ImGui::InputFloat3("Rotation", (float*)&Rotation);
		ImGui::InputFloat3("Location", (float*)&Location);

		Vector3 CurSliderScale{ 0,0,0 }, CurSliderRotation{ 0,0,0 }, CurSliderLocation{ 0,0,0 };
		ImGui::SliderFloat3("_Scale",(float*) &CurSliderScale, -0.1f, +0.1f); 
		ImGui::SliderFloat3("_Rotation", (float*)&CurSliderRotation , -0.1f, +0.1f);
		ImGui::SliderFloat3("_Location", (float*)&CurSliderLocation, -0.1f, +0.1f);

		SetScale(Scale + CurSliderScale);
		SetRotation(Rotation + CurSliderRotation);
		SetLocation(Location + CurSliderLocation);



		ImGui::TreePop();
		ImGui::Separator();
	}
};

const Matrix& Engine::Transform::UpdateWorld()&
{
	World = FMath::WorldMatrix(Scale,Rotation,Location);
	return World;
};

void Engine::Transform::Rotate(const Vector3& Rotation, const float DeltaTime)&
{
	this->Rotation.y += Rotation.y * DeltaTime;
	this->Rotation.x += Rotation.x * DeltaTime;
	this->Rotation.z += Rotation.z * DeltaTime;
	Matrix RotationMatrix = FMath::Rotation(Rotation);
	std::memcpy(&Forward, &RotationMatrix._31, sizeof(Vector3));
	std::memcpy(&Right, &RotationMatrix._11, sizeof(Vector3));
	std::memcpy(&Up, &RotationMatrix._21, sizeof(Vector3));

	/*Up = FMath::MulNormal(Vector3{ 0,1,0 }, RotationMatrix);
	Forward = FMath::MulNormal(Vector3{ 0,0,1 }, RotationMatrix);
	Right = FMath::MulNormal(Vector3{ 1,0,0 }, RotationMatrix);*/
}

void Engine::Transform::RotateYaw(const float Radian, const float DeltaTime)&
{
	Rotation.y += Radian * DeltaTime;
	Matrix RotationMatrix = FMath::Rotation(Rotation);
	
	std::memcpy(&Forward, &RotationMatrix._31, sizeof(Vector3));
	std::memcpy(&Right, &RotationMatrix._11, sizeof(Vector3));
	std::memcpy(&Up, &RotationMatrix._21, sizeof(Vector3));
	/*Up = FMath::MulNormal(Vector3{ 0,1,0 }, RotationMatrix);
	Forward = FMath::MulNormal(Vector3{ 0,0,1 }, RotationMatrix);
	Right = FMath::MulNormal(Vector3{ 1,0,0 }, RotationMatrix);*/
}

void Engine::Transform::RotateRoll(const float Radian, const float DeltaTime)&
{
	Rotation.z += Radian * DeltaTime;
	Matrix RotationMatrix = FMath::Rotation(Rotation);
	std::memcpy(&Forward, &RotationMatrix._31, sizeof(Vector3));
	std::memcpy(&Right, &RotationMatrix._11, sizeof(Vector3));
	std::memcpy(&Up, &RotationMatrix._21, sizeof(Vector3));
	/*Up = FMath::MulNormal(Vector3{ 0,1,0 }, RotationMatrix);
	Forward = FMath::MulNormal(Vector3{ 0,0,1 }, RotationMatrix);
	Right = FMath::MulNormal(Vector3{ 1,0,0 }, RotationMatrix);*/
}

void Engine::Transform::RotatePitch(const float Radian, const float DeltaTime)&
{
	Rotation.x += Radian * DeltaTime;
	Matrix RotationMatrix = FMath::Rotation(Rotation);
	std::memcpy(&Forward, &RotationMatrix._31, sizeof(Vector3));
	std::memcpy(&Right, &RotationMatrix._11, sizeof(Vector3));
	std::memcpy(&Up, &RotationMatrix._21, sizeof(Vector3));

	/*Up = FMath::MulNormal(Vector3{ 0,1,0 }, RotationMatrix);
	Forward = FMath::MulNormal(Vector3{ 0,0,1 }, RotationMatrix);
	Right = FMath::MulNormal(Vector3{ 1,0,0 }, RotationMatrix);*/
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
	(Vector3 ToEnd, 
	const float DeltaTime,
	const float Speed)
{
	ToEnd = FMath::Normalize(ToEnd);
	Location += ToEnd * DeltaTime * Speed;
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

 void Engine::Transform::SetRotation(const Vector3& Rotation)&
{
     this->Rotation = Rotation;
	 Matrix RotationMatrix = FMath::Rotation(Rotation);
	 std::memcpy(&Forward, &RotationMatrix._31, sizeof(Vector3));
	 std::memcpy(&Right, &RotationMatrix._11, sizeof(Vector3));
	 std::memcpy(&Up, &RotationMatrix._21, sizeof(Vector3));
	 /*Up = FMath::MulNormal(Vector3{ 0,1,0 }, RotationMatrix);
	 Forward = FMath::MulNormal(Vector3{ 0,0,1 }, RotationMatrix);
	 Right = FMath::MulNormal(Vector3{ 1,0,0 }, RotationMatrix);*/
};


