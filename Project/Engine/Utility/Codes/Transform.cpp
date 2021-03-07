#include "Camera.h"
#include "Transform.h"
#include "FMath.hpp"
#include "imgui.h"
#include <string>
#include "StringHelper.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include "FileHelper.h"
#include "UtilityGlobal.h"
#include <fstream>
#include <ostream>
#include "ResourceSystem.h"

void Engine::Transform::Initialize(const std::string& OwnerClassIdentifier)&
{
	Super::Initialize();
	this->OwnerClassIdentifier = OwnerClassIdentifier;
	UpdateWorld();
	Load();
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
		if (ImGui::Button("Save"))
		{
			Save();
		}

		ImGui::InputFloat("LocationSensitivity", &_EditProperty.LocationSensitivity);
		ImGui::InputFloat3("Scale", (float*)&Scale);
		ImGui::InputFloat3("Rotation", (float*)&Rotation);
		ImGui::InputFloat3("Location", (float*)&Location);

		Vector3 CurSliderScale{ 0,0,0 }, CurSliderRotation{ 0,0,0 }, CurSliderLocation{ 0,0,0 };
		ImGui::SliderFloat3("_Scale",(float*) &CurSliderScale, -0.1f, +0.1f); 
		ImGui::SliderFloat3("_Rotation", (float*)&CurSliderRotation , -0.01f, +0.01f);
		ImGui::SliderFloat3("_Location", (float*)&CurSliderLocation, 
			-_EditProperty.LocationSensitivity, +_EditProperty.LocationSensitivity);

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

	if (AttachBoneToRoot)
	{
		World *= (*AttachBoneToRoot);
	};

	if (OwnerTransform)
	{
		World *= (*OwnerTransform);
	};

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




 void Engine::Transform::Save()&
 {
	 using namespace rapidjson;

	 StringBuffer StrBuf;
	 PrettyWriter<StringBuffer> Writer(StrBuf);
	 
	 Writer.StartObject();

	 Writer.Key("Transform");
	 Writer.StartObject();
	 {
		 Writer.Key("Scale");
		 Writer.StartArray();
		 Writer.Double(Scale.x);
		 Writer.Double(Scale.y);
		 Writer.Double(Scale.z);
		 Writer.EndArray();

		 Writer.Key("Rotation");
		 Writer.StartArray();
		 Writer.Double(Rotation.x);
		 Writer.Double(Rotation.y);
		 Writer.Double(Rotation.z);
		 Writer.EndArray();
	 }
	 Writer.EndObject();


	 Writer.EndObject();
	 const std::filesystem::path TargetPath = Engine::Global::ResourcePath 
		 / "Transform" / (OwnerClassIdentifier + ".json");
	 std::ofstream Of{ TargetPath };
	 Of << StrBuf.GetString();
 }

 void Engine::Transform::Load()&
 {
	 const std::string LoadProp = "Transform_" + OwnerClassIdentifier;
	 auto& ResourceSys = ResourceSystem::Instance;

	 std::optional<TransformProperty> IsOffset = ResourceSys->GetAny<TransformProperty>(ToW(LoadProp));

	 if (IsOffset)
	 {
		 SetScale(IsOffset->Scale);
		 SetRotation(IsOffset->Rotation);
	 }
	 else
	 {
		 const std::filesystem::path TargetPath = Engine::Global::ResourcePath / "Transform" / (OwnerClassIdentifier + ".json");
		 std::ifstream Is{ TargetPath };
		 using namespace rapidjson;
		 if (!Is.is_open()) return;

		 IStreamWrapper Isw(Is);
		 Document _Document;
		 _Document.ParseStream(Isw);

		 if (_Document.HasParseError())
		 {
			 MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
			 return;
		 }

		 const Value& AnimationJsonTable = _Document["Transform"];

		 const auto& ScaleArr = AnimationJsonTable.FindMember("Scale")->value.GetArray();

		 SetScale({
			 ScaleArr[0].GetFloat()  ,
			 ScaleArr[1].GetFloat()  ,
			 ScaleArr[2].GetFloat() });

		 const auto& RotationArr = AnimationJsonTable.FindMember("Rotation")->value.GetArray();

		 SetRotation({
			 RotationArr[0].GetFloat() ,
			 RotationArr[1].GetFloat() ,
			 RotationArr[2].GetFloat() });

		 TransformProperty CurSaveClassProperty{};
		 CurSaveClassProperty.Scale = Scale;
		 CurSaveClassProperty.Rotation = Rotation;
		 ResourceSys->InsertAny(ToW(LoadProp), CurSaveClassProperty);
	 };
 }

 void Engine::Transform::AttachBone(const Matrix* const TargetBoneToRoot)&
 {
	 AttachBoneToRoot = TargetBoneToRoot;
 }


 void Engine::Transform::AttachTransform(const Matrix* const TargetParentTransform)&
 {
	 OwnerTransform = TargetParentTransform;

 }

