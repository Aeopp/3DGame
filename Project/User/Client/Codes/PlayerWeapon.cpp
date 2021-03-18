#include "..\\stdafx.h"
#include "PlayerWeapon.h"
#include "Transform.h"
#include "StaticMesh.h"
#include <iostream>
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"
#include "NormalLayer.h"

void PlayerWeapon::Initialize(
	const std::optional<Vector3>& SpawnScale ,
	const std::optional<Vector3>& SpawnRotation,
	const Vector3& SpawnLocation )&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>(typeid(PlayerWeapon).name() );

	if (SpawnScale)
	{
		_Transform->SetScale(*SpawnScale);
	}
	if (SpawnRotation)
	{
		_Transform->SetRotation(*SpawnRotation);
	}
	
	_Transform->SetLocation(SpawnLocation);

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(L"PlayerWeapon");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::PlayerAttack, _Transform ,
			typeid(PlayerWeapon).name());
	_Collision->RenderObjectTransform = _Transform;

	// 바운딩 박스.
	{
		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
		D3DXComputeBoundingBox(_StaticMesh->LocalVertexLocations->data(),
			_StaticMesh->LocalVertexLocations->size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
			(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
	}

	_StaticMesh->SetUpCullingInformation(_Collision->_Geometric->LocalSphere ,
		_Transform);
	_StaticMesh->bCullingOn = true;

	// 바운딩 스피어
	{
		/*Vector3 BoundingSphereCenter;
		float BoundingSphereRadius;
		D3DXComputeBoundingSphere(_StaticMesh->GetVertexLocations().data(), _StaticMesh->GetVertexLocations().size(),
			sizeof(Vector3), &BoundingSphereCenter, &BoundingSphereRadius);

		_Collision->_Geometric = std::make_unique<Engine::GSphere>(BoundingSphereRadius, BoundingSphereCenter);
		static_cast<Engine::GSphere* const>(_Collision->_Geometric.get())->MakeDebugCollisionSphere(Device);*/
	}

	_Collision->RefCollisionables().insert(
		{
			Engine::CollisionTag::Enemy 
		});

	_Collision->RefPushCollisionables().insert(
		{

		});

	_Collision->bCollision = false;


}

void PlayerWeapon::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();
	this->Device = Device;
	bCapturable = true;
	auto _StaticMeshProto = std::make_shared<Engine::StaticMesh>();

	_StaticMeshProto->Load<Vertex::LocationTangentUV2D>(Device,
		App::ResourcePath / L"Mesh" / L"StaticMesh" / L"",
		L"PlayerWeapon.fbx", L"PlayerWeapon" ,
		Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_StaticMeshProto)>(L"PlayerWeapon", _StaticMeshProto);
}

void PlayerWeapon::Event()&
{
	Super::Event();


	if (Engine::Global::bDebugMode)
	{
		auto* const _StaticMesh = GetComponent<Engine::StaticMesh>();
		if (_StaticMesh->_DissolveInfo)
		{
			ImGui::SliderFloat("SliceAmout", &_StaticMesh->_DissolveInfo->SliceAmount, 0.0f, 1.f);
			ImGui::SliderFloat("BurnSize", &_StaticMesh->_DissolveInfo->BurnSize, 0.0f, 1.f);
			ImGui::SliderFloat("EmissionAmount", &_StaticMesh->_DissolveInfo->EmissionAmount, 0.0f, 10.f);
		}
	}
}


void PlayerWeapon::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto* const _StaticMesh = GetComponent<Engine::StaticMesh>();

	if (_StaticMesh->_DissolveInfo)
	{
		_StaticMesh->_DissolveInfo->SliceAmount += DeltaTime * SliceAmountSpeed;

		if (_StaticMesh->_DissolveInfo->SliceAmount >  ( 1.0f + DeltaTime * SliceAmountSpeed))
		{
			_StaticMesh->_DissolveInfo = std::nullopt;
		}
	}
};

void PlayerWeapon::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
};

void PlayerWeapon::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void PlayerWeapon::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
}

std::function<Engine::Object::SpawnReturnValue(const Engine::Object::SpawnParam&)> 
PlayerWeapon::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;

	static bool SpawnSelectCheck = false;

	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);
	if (SpawnSelectCheck)
	{
		return[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, PlayerWeapon>
				(L"Static", L"PlayerWeapon_" + std::to_wstring(SpawnID++),
					std::nullopt, std::nullopt, 
					SpawnParams.Location);

			return Engine::Object::SpawnReturnValue{};
		};
	}
	else
	{
		return {};
	}
}

void PlayerWeapon::DissolveStart(const float SliceAmountSpeed, const float SliceAmoutStart)&
{
	auto* const _StaticMesh = GetComponent<Engine::StaticMesh>();
	Engine::Mesh::DissolveInfo DissolveInfo;
	DissolveInfo.BurnSize = 0.3f;
	DissolveInfo.EmissionAmount = 3.f;
	DissolveInfo.SliceAmount = SliceAmoutStart;
	_StaticMesh->_DissolveInfo = DissolveInfo;
	this->SliceAmountSpeed = SliceAmountSpeed;
};

void PlayerWeapon::StartAttack(Engine::Object* const AttackOwner, const float Force, const float ForceJump)&
{
	GetComponent<Engine::Collision>()->bCollision = true;
	this->AttackOwner = AttackOwner;
	this->ForceJump = ForceJump;
	this->Force = Force;
}

void PlayerWeapon::EndAttack(Engine::Object* const AttackOwner)&
{
	GetComponent<Engine::Collision>()->bCollision = false;
	this->AttackOwner = AttackOwner;
	this->ForceJump = 0.0f;
	this->Force = 0.0f;
};


std::shared_ptr<Engine::Object> PlayerWeapon::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)>  Clone = *this;
	return std::make_shared<PlayerWeapon>(Clone);
}

std::optional<Engine::Object::SpawnReturnValue> 
PlayerWeapon::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	Initialize(_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue{} };
}
