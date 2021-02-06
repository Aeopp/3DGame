#include "..\\stdafx.h"
#include "Player.h"
#include "Transform.h"
#include "DynamicMesh.h"
#include <iostream>
#include "DynamicMesh.h"
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


void Player::Initialize(
	const Vector3& Scale,
	const Vector3& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>();
	_Transform->SetScale(Scale);
	_Transform->SetRotation(Rotation);
	_Transform->SetLocation(SpawnLocation);
	
	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"Player");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Decorator, _Transform);
	
	// 바운딩 박스.
	{
		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
		D3DXComputeBoundingBox(_SkeletonMesh->LocalVertexLocations->data(),
			                   _SkeletonMesh->LocalVertexLocations->size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
			(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox(Device);
	}

	RenderInterface::SetUpCullingInformation(
		_Collision->_Geometric->LocalSphere  ,
		_Transform);

	RenderInterface::bCullingOn = false;
	_SkeletonMesh->PlayAnimation(0u,1.0);
	
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

void Player::PrototypeInitialize(IDirect3DDevice9* const Device,
						const Engine::RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device,_Group);
	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::Skeleton>(Device, 
		App::ResourcePath/L"Mesh"/L"DynamicMesh"/L"PlayerXfile"/L"",
		L"Player.X", L"Player");

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>(L"Player", _SkeletonMeshProto);
}

void Player::Event()&
{
	Super::Event();

	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
	ImGui::Begin("Animation");
	static int32 AnimIndex = 0u;
	static float Acceleration = 1.f;
	ImGui::SliderInt("Index : %d", &AnimIndex, 0, 50);
	ImGui::SliderFloat("Acceleration : %f", &Acceleration, 1.f, 1000.f);
	if (ImGui::Button("Play"))
	{
		_SkeletonMeshComponent->PlayAnimation(AnimIndex, static_cast<double>(Acceleration));
	}
	ImGui::End();
}

void Player::Render()&
{
	Super::Render();
	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	Device->SetTransform(D3DTS_WORLD, &World);
	_SkeletonMeshComponent->Render();
}

void Player::Update(const float DeltaTime)&
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

void Player::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
};

void Player::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void Player::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
};
