#include "..\\stdafx.h"
#include "Belatos.h"
#include "Transform.h"
#include "DynamicMesh.h"
#include <iostream>
#include "Management.h"
#include "DynamicMesh.h"
#include "ThirdPersonCamera.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Management.h"
#include "Vertexs.hpp" 
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"
#include "EnemyLayer.h"
#include "NavigationMesh.h"


void Belatos::FSM(const float DeltaTime)&
{
	Super::FSM(DeltaTime);

	auto& Control = RefControl();
	auto _Transform = GetComponent<Engine::Transform>();
	auto _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();

	FSMControlInformation CurrentFSMControlInfo{ _Transform,  _SkeletonMesh ,DeltaTime };

	switch (CurrentState)
	{
	case Belatos::State::Wait:
		WaitState(CurrentFSMControlInfo);
		break;
	default:
		break;
	};
}

void Belatos::Edit()&
{
	Super::Edit();
		// ImGui·Î º¯¼ö ÅëÁ¦ . 
	if (Engine::Global::bDebugMode)
	{
		auto _Transform = GetComponent<Engine::Transform>();
		auto& _Physic = _Transform->RefPhysic();
		ImGui::SliderFloat("LandCheckHighRange", &LandCheckHighRange, 0.f ,30.f);
	}
}


void Belatos::HitNotify(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
}

void Belatos::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
}

void Belatos::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
}
void Belatos::Event()&
{
	Super::Event();
}

std::shared_ptr<Engine::Object> Belatos::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)> Clone = *this;
	return std::make_shared<MyType>(Clone);
}

std::optional<Engine::Object::SpawnReturnValue> Belatos::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	MyType::Initialize(
		_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue {} };
}

void Belatos::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

}
void Belatos::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);

}
void Belatos::WaitTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Wait_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Belatos::State::Wait;
}

void Belatos::WaitState(const FSMControlInformation& FSMControlInfo)&
{
	// ¶Û¼ö ÀÖÀ¸¸é ¶Ù¾îºÁ¶ó 
}


void Belatos::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::MonsterPrototypeInitialize(Device);

	bCapturable = true;
	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>
		(Device, App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"",
			L"Belatos.fbx", L"Belatos",
			Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>
		(L"Belatos", _SkeletonMeshProto);
}
void Belatos::Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&
{
	Super::MonsterInitialize(Scale, Rotation, SpawnLocation);

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

	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"Belatos");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Enemy, _Transform,
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
	FSMControlInformation InitFSMControlInfo{ _Transform,_SkeletonMesh,1.f };
	WaitTransition(InitFSMControlInfo);

	_Collision->RefCollisionables().insert({
		Engine::CollisionTag::Player,
		Engine::CollisionTag::PlayerAttack
		});

	_Collision->RefPushCollisionables().insert({
		Engine::CollisionTag::Player
		});

	auto& _NaviMesh = RefNaviMesh();
	Vector2 SpawnLocation2D = { SpawnLocation.x , SpawnLocation.z };

	Engine::Transform::PhysicInformation InitPhysic;
	InitPhysic.Gravity = 220.f;
	InitPhysic.bGravityEnable = true;
	InitPhysic.Velocity = { 0,0,0 };
	InitPhysic.Acceleration = { 0,0,0 };

	CurrentCell = _NaviMesh.GetCellFromXZLocation(SpawnLocation2D);
	const Vector3 Location = _Transform->GetLocation();
	auto bCellResult = CurrentCell->Compare(Location);
	if (bCellResult)
	{
		const Engine::Cell::CompareType _CompareType = bCellResult->_Compare;
		CurrentCell = bCellResult->Target;
		InitPhysic.CurrentGroundY = bCellResult->ProjectLocation.y;
	}

	_Transform->EnablePhysic(InitPhysic);



}
;

std::function < Engine::Object::SpawnReturnValue
(const Engine::Object::SpawnParam&)>
Belatos::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;
	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);

	if (SpawnSelectCheck)
	{
		return[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<EnemyLayer, MyType>
				(L"Static", L"Belatos_" + std::to_wstring(SpawnID++),
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
}

