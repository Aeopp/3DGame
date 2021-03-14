#include "..\\stdafx.h"
#include "Monster.h"
#include "Transform.h"
#include "Player.h"
#include "DynamicMesh.h"
#include <iostream>
#include "Management.h"
#include "DynamicMesh.h"
#include "ThirdPersonCamera.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Management.h"
#include "Vertexs.hpp" 
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"
#include "NPC.h"
#include "NormalLayer.h"
#include "Collision.h"

void NPC::Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&
{
	auto _Transform = AddComponent<Engine::Transform>(typeid(MyType).name());

	if (Scale)
	{
		_Transform->SetScale(*Scale);
	}

	if (Rotation)
	{
		_Transform->SetRotation(*Rotation);
	}

	_Transform->SetLocation(SpawnLocation);

	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"NPC");
	_SkeletonMesh->bMotionBlur = false;
	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::NPC, _Transform,
			typeid(MyType).name());

	_Collision->RenderObjectTransform = _Transform;

	{
		Vector3 BoundingBoxMin{}, BoundingBoxMax{};

		D3DXComputeBoundingBox(_SkeletonMesh->LocalVertexLocations->data(),
			_SkeletonMesh->LocalVertexLocations->size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
			(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
	}

	{

	}

	_SkeletonMesh->SetUpCullingInformation(_Collision->_Geometric->LocalSphere,
		_Transform);
	_SkeletonMesh->bCullingOn = true;

	_Collision->RefCollisionables().insert({
		Engine::CollisionTag::Player
		});

	_Collision->RefPushCollisionables().insert({
		});

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Take 001";
	_SkeletonMesh->PlayAnimation(_AnimNotify);
}

void NPC::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();

	bCapturable = true;
	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>
		(Device, App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"",
			L"NPC.fbx", L"NPC",
			Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>
		(L"NPC", _SkeletonMeshProto);
}

std::shared_ptr<Engine::Object> NPC::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)> Clone = *this;
	return std::make_shared<MyType>(Clone);
}

std::optional<Engine::Object::SpawnReturnValue> NPC::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	MyType::Initialize(
		_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue {} };
}

std::function < Engine::Object::SpawnReturnValue
(const Engine::Object::SpawnParam&)>
NPC::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;
	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);

	if (SpawnSelectCheck)
	{
		return[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, MyType>
				(L"Static", L"NPC_" + std::to_wstring(SpawnID++),
					std::nullopt,
					std::nullopt,
					SpawnParams.Location);

			return Engine::Object::SpawnReturnValue{};
		};
	}
	else
	{
		return {};
	}

	return std::function<SpawnReturnValue(const SpawnParam&)>();
};

void NPC::Event()&
{
	Super::Event();
	Edit();
}

void NPC::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}

void NPC::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
}

void NPC::Edit()&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::SliderFloat3("ViewOffsetLocation", ViewLocationOffset, -100.f, 100.f);
	}
}

void NPC::HitNotify(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);

	auto* _Player= dynamic_cast<Player* const>(Target);
	if (_Player)
	{
		
	}
}
