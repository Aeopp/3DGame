#include "Collision.h"
#include "Object.h"
#include "Transform.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "imgui.h"
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

void Engine::Collision::Render(Engine::Renderer* const _Renderer)&
{
	const Matrix ToWorld = OffsetMatrix * OwnerTransform->UpdateWorld();
	Device->SetTransform(D3DTS_WORLD, &ToWorld);
	_Geometric->Render(Device, bCurrentFrameCollision);
}

void Engine::Collision::Initialize(
	IDirect3DDevice9* const Device,
	const CollisionTag _Tag,
	class Transform* const OwnerTransform  , 
	const std::string& ClassIdentifier)&
{
	Super::Initialize();
	this->Device = Device;
	this->_Tag = _Tag;
	this->OwnerTransform = OwnerTransform;
	SetUpRenderingInformation(RenderInterface::Group::DebugCollision);
	RenderInterface::bCullingOn = false;
	OwnerClassIdentifier = ClassIdentifier;
	Load();
};

void Engine::Collision::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	bCurrentFrameCollision = false;
	this->Owner = Owner;

	if (Owner->IsPendingKill())
	{
		CollisionSystem::Instance->UnRegist(_Tag, this);
	}
	else
	{
		CollisionSystem::Instance->Regist(_Tag, this);
	}
	
	_Geometric->Update(OffsetMatrix * OwnerTransform->UpdateWorld());

	bImmobility = false;
	CurrentCheckedCollisionIDs.clear();
};

void Engine::Collision::Event(Object* Owner)&
{
	RenderInterface::Regist();

	if (Engine::Global::bDebugMode)
	{
		if (ImGui::TreeNode(("CollisionEdit_" + ToA(Owner->GetName())).c_str()))
		{
			if (ImGui::Button("Save"))
			{
				Save();
			}

			if (ImGui::TreeNode("Offset"))
			{
				ImGui::InputFloat3("Scale", (float*)&_OffsetInfo.Scale);
				ImGui::InputFloat3("Rotation", (float*)&_OffsetInfo.Rotation);
				ImGui::InputFloat3("Location", (float*)&_OffsetInfo.Location);

				Vector3 CurSliderScale{ 0,0,0 }, CurSliderRotation{ 0,0,0 }, CurSliderLocation{ 0,0,0 };
				ImGui::SliderFloat3("_Scale", (float*)&CurSliderScale, -0.01f, +0.01f);
				ImGui::SliderFloat3("_Rotation", (float*)&CurSliderRotation, -0.01f, +0.01f);
				ImGui::SliderFloat3("_Location", (float*)&CurSliderLocation, -0.1f, +0.1f);

				_OffsetInfo.Scale += CurSliderScale;
				_OffsetInfo.Rotation += CurSliderRotation;
				_OffsetInfo.Location += CurSliderLocation;

				OffsetMatrix = FMath::WorldMatrix(_OffsetInfo.Scale, _OffsetInfo.Rotation, _OffsetInfo.Location);

				ImGui::TreePop();
			}

			ImGui::TreePop();
			ImGui::Separator();
		}

	}
}


bool Engine::Collision::IsCollision(Collision* const Rhs)&
{
	const auto CollisionInfo = _Geometric->IsCollision(Rhs->_Geometric.get());

	if (CollisionInfo)
	{
		const auto [CrossArea, PushDir] = *CollisionInfo;

		bCurrentFrameCollision = true;

		if (PushCollisionables.contains(Rhs->_Tag) && false == Rhs->bImmobility)
		{
			if (Rhs->PushCollisionables.contains(_Tag) && false == bImmobility)
			{
				Rhs->OwnerTransform->SetLocation(
					Rhs->OwnerTransform->GetLocation() + PushDir * CrossArea * 0.5f);
				
				Rhs->_Geometric->Update(
					Rhs->OffsetMatrix * Rhs->OwnerTransform->UpdateWorld() );

				OwnerTransform->SetLocation(
					OwnerTransform->GetLocation() + -PushDir * CrossArea * 0.5f);

				_Geometric->Update(OffsetMatrix* OwnerTransform->UpdateWorld());
			}
			else
			{
				Rhs->OwnerTransform->SetLocation(
					Rhs->OwnerTransform->GetLocation() + PushDir * CrossArea);

				Rhs->_Geometric->Update(  
					Rhs->OffsetMatrix * Rhs->OwnerTransform->UpdateWorld() );
			}
		}
		if (!CurrentCheckedCollisionIDs.contains(Rhs->ID))
		{
			CurrentCheckedCollisionIDs.insert(Rhs->ID);
			Rhs->CurrentCheckedCollisionIDs.insert(ID);

			auto iter = HitCollisionIDs.find(Rhs->ID);

			if (iter == std::end(HitCollisionIDs))
			{
				HitCollisionIDs.insert(iter, Rhs->ID);
				Rhs->HitCollisionIDs.insert(ID);
				Owner->HitBegin(Rhs->Owner, PushDir, CrossArea);
				Rhs->Owner->HitBegin(Owner, -PushDir, CrossArea);
			}
			else
			{
				Owner->HitNotify(Rhs->Owner, PushDir, CrossArea);
				Rhs->Owner->HitNotify(Owner, -PushDir, CrossArea);
			}
		}
	}

	return CollisionInfo.has_value();
};

void Engine::Collision::Save()&
{
	using namespace rapidjson;

	StringBuffer StrBuf;
	PrettyWriter<StringBuffer> Writer(StrBuf);
	// Cell Information Write...
	Writer.StartObject();

	Writer.Key("Offset");
	Writer.StartObject();
	{
		Writer.Key("Scale");
		Writer.StartArray();
		Writer.Double(_OffsetInfo.Scale.x);
		Writer.Double(_OffsetInfo.Scale.y);
		Writer.Double(_OffsetInfo.Scale.z);
		Writer.EndArray();

		Writer.Key("Rotation");
		Writer.StartArray();
		Writer.Double(_OffsetInfo.Rotation.x);
		Writer.Double(_OffsetInfo.Rotation.y);
		Writer.Double(_OffsetInfo.Rotation.z);
		Writer.EndArray();

		Writer.Key("Location");
		Writer.StartArray();
		Writer.Double(_OffsetInfo.Location.x);
		Writer.Double(_OffsetInfo.Location.y);
		Writer.Double(_OffsetInfo.Location.z);
		Writer.EndArray();
	}
	Writer.EndObject();


	Writer.EndObject();
	const std::filesystem::path TargetPath = Engine::Global::ResourcePath / "Collision" / (OwnerClassIdentifier + ".json");
	std::ofstream Of{ TargetPath };
	Of << StrBuf.GetString();
}; 

void Engine::Collision::Load()&
{
	const std::string LoadProp = "Collision_" + OwnerClassIdentifier;
	auto& ResourceSys = ResourceSystem::Instance;

	std::optional<OffsetInformation> IsOffset = ResourceSys->GetAny<OffsetInformation>(ToW(LoadProp));

	if (IsOffset)
	{
		_OffsetInfo = *IsOffset;

	}
	else
	{
		const std::filesystem::path TargetPath = Engine::Global::ResourcePath / "Collision" / (OwnerClassIdentifier + ".json");
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

		const Value& AnimationJsonTable = _Document["Offset"];

		const auto& ScaleArr = AnimationJsonTable.FindMember("Scale")->value.GetArray();
		_OffsetInfo.Scale.x  = ScaleArr[0].GetFloat();
		_OffsetInfo.Scale.y  = ScaleArr[1].GetFloat();
		_OffsetInfo.Scale.z  = ScaleArr[2].GetFloat();


		const auto& RotationArr = AnimationJsonTable.FindMember("Rotation")->value.GetArray();
		_OffsetInfo.Rotation.x = RotationArr[0].GetFloat();
		_OffsetInfo.Rotation.y = RotationArr[1].GetFloat();
		_OffsetInfo.Rotation.z = RotationArr[2].GetFloat();


		const auto& LocationArr = AnimationJsonTable.FindMember("Location")->value.GetArray();
		_OffsetInfo.Location.x =  LocationArr[0].GetFloat();
		_OffsetInfo.Location.y =  LocationArr[1].GetFloat();
		_OffsetInfo.Location.z =  LocationArr[2].GetFloat();

		ResourceSys->InsertAny(ToW(LoadProp) , _OffsetInfo);
	}

	OffsetMatrix = FMath::WorldMatrix(_OffsetInfo.Scale, _OffsetInfo.Rotation, _OffsetInfo.Location);
};

