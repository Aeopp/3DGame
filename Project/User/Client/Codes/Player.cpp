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
	_SkeletonMesh->PlayAnimation(0u,100.f, -1);
	
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

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>(Device,
		App::ResourcePath/L"Mesh"/L"DynamicMesh"/L"",
		L"GolemAnim.fbx", L"Player");

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>(L"Player", _SkeletonMeshProto);
}

void Player::Event()&
{
	Super::Event();

	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
}

void Player::Render(const Matrix& View,
					const Matrix& Projection,
					const Vector4& CameraLocation)&
{
	Super::Render(View ,Projection ,CameraLocation);
	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	_SkeletonMeshComponent->Render(World ,View ,Projection, CameraLocation);
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

	static float TransitionDuration = 1.0;
	static float TransitionAceeleration = 10.0; 
	ImGui::SliderFloat("TransitionDuration", &TransitionDuration, 0.1, 10.0);
	ImGui::SliderFloat("TransitionAcceleration", & TransitionAceeleration , 1.0 , 1000.0);

	if (ImGui::Button("Anim 0"))
	{
		GetComponent<Engine::SkeletonMesh>()->PlayAnimation(0u, TransitionAceeleration, TransitionDuration);
	}
	if (ImGui::Button("Anim 1"))
	{
		GetComponent<Engine::SkeletonMesh>()->PlayAnimation(1u, TransitionAceeleration, TransitionDuration);
	}
	if (ImGui::Button("Anim 2"))
	{
		GetComponent<Engine::SkeletonMesh>()->PlayAnimation(2u, TransitionAceeleration, TransitionDuration);
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
