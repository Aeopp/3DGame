#include "..\\stdafx.h"
#include "TombStone.h"
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

void TombStone::Initialize(
	const std::optional<Vector3>& SpawnScale ,
	const std::optional<Vector3>& SpawnRotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>(typeid(TombStone).name() );

	if (SpawnScale)
	{
		_Transform->SetScale(*SpawnScale);
	}
	if (SpawnRotation)
	{
		_Transform->SetRotation(*SpawnRotation);
	}
	
	_Transform->SetLocation(SpawnLocation);

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(L"Floor");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Decorator, _Transform ,
			typeid(TombStone).name());

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
			Engine::CollisionTag::Decorator
		});

	_Collision->RefPushCollisionables().insert(
		{
	          Engine::CollisionTag::Decorator
		});
}

void TombStone::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();
	this->Device = Device;

	auto _StaticMeshProto = std::make_shared<Engine::StaticMesh>();

	_StaticMeshProto->Load<Vertex::LocationTangentUV2D>(Device,
		App::ResourcePath / L"Mesh" / L"StaticMesh" / L"SnowTerrain" / L"",
		L"SnowTerrain.dae", L"Floor" ,
		Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_StaticMeshProto)>(L"Floor", _StaticMeshProto);
}

void TombStone::Event()&
{
	Super::Event();
}


void TombStone::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto& Control = RefControl();
	auto _Transform = GetComponent<Engine::Transform>();
	static constexpr float Speed = 10.f;

	if (Control.IsPressing(DIK_UP))
	{
		_Transform->Move({ 0,0,1 }, DeltaTime, Speed);
	}
	if (Control.IsPressing(DIK_DOWN))
	{
		_Transform->Move({ 0,0,1 },DeltaTime, -Speed);
	}
	if (Control.IsPressing(DIK_LEFT))
	{
		_Transform->Move({1,0,0} , DeltaTime, -Speed);
	}
	if (Control.IsPressing(DIK_RIGHT))
	{
		_Transform->Move({1,0,0},DeltaTime, Speed);
	}
	if (Control.IsPressing(DIK_PGUP))
	{
		_Transform->Move({ 0,1,0 },DeltaTime, Speed);
	}
	if (Control.IsPressing(DIK_PGDN))
	{
		_Transform->Move({ 0,1,0 },  DeltaTime, -Speed);
	}


	if (Control.IsPressing(DIK_R))
	{
		_Transform->RotateYaw(Speed, DeltaTime);
	}
	if (Control.IsPressing(DIK_T))
	{
		_Transform->RotateYaw(-Speed, DeltaTime);
	}
	if (Control.IsPressing(DIK_F))
	{
		_Transform->RotatePitch(Speed, DeltaTime); 
	}
	if (Control.IsPressing(DIK_G))
	{
		_Transform->RotatePitch(-Speed, DeltaTime);
	}
	if (Control.IsPressing(DIK_V))
	{
		_Transform->RotateRoll(Speed, DeltaTime);
	}
	if (Control.IsPressing(DIK_B))
	{
		_Transform->RotateRoll(-Speed, DeltaTime);
	}
};

void TombStone::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
};

void TombStone::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void TombStone::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
}
std::function<Engine::Object::SpawnReturnValue(const Engine::Object::SpawnParam&)> TombStone::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;

	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);
	if (SpawnSelectCheck)
	{
		return[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, TombStone>
				(L"Static", L"TombStone_" + std::to_wstring(SpawnID++),
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

std::shared_ptr<Engine::Object> TombStone::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)>  Clone = *this;
	return std::make_shared<TombStone>(Clone);
}

std::optional<Engine::Object::SpawnReturnValue> TombStone::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	Initialize(_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue{} };
}
