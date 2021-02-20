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
	const Vector3& Scale,
	const Vector3& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>();
	_Transform->SetScale({ 1,1,1 });
	_Transform->SetScale(Scale);
	_Transform->SetRotation(Rotation);
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

	RenderInterface::SetUpCullingInformation(
		_Collision->_Geometric->LocalSphere  ,
		_Transform);

	RenderInterface::bCullingOn = true;

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

void TombStone::PrototypeInitialize(IDirect3DDevice9* const Device,
						const Engine::RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device,_Group);
	this->Device = Device;

	auto _StaticMeshProto = std::make_shared<Engine::StaticMesh>();

	_StaticMeshProto->Load<Vertex::LocationTangentUV2D>(Device,
		App::ResourcePath / L"Mesh" / L"StaticMesh" / L"SnowTerrain" / L"",
		L"SnowTerrain.dae", L"Floor");

	RefResourceSys().InsertAny<decltype(_StaticMeshProto)>(L"Floor", _StaticMeshProto);
}

void TombStone::Event()&
{
	Super::Event();
}

void TombStone::Render(const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation)&
{
	Super::Render( View ,Projection ,CameraLocation);
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	auto _StaticMesh = GetComponent<Engine::StaticMesh>();
	_StaticMesh->Render(World , View ,Projection ,CameraLocation);
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
void TombStone::PrototypeEdit()&
{
	auto& RefManager = Engine::Management::Instance;

	static uint32 SpawnID = 0u;

	if (ImGui::Button("Spawn"))
	{
		RefManager->NewObject<Engine::NormalLayer, TombStone>(L"Static", L"TombStone_" + std::to_wstring(SpawnID++),
			Vector3{ 1.f,1.f,1.f }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 });
	}
}
