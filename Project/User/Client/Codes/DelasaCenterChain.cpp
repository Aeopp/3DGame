#include "..\\stdafx.h"
#include "DelasaCenterChain.h"
#include "Transform.h"
#include "DynamicMesh.h"
#include <iostream>
#include "DynamicMesh.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "imgui.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"
#include "NormalLayer.h"
#include "Collision.h"


void DelasaCenterChain::Initialize(
	const std::optional<Vector3>& Scale,
	const std::optional<Vector3>& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>(typeid(DelasaCenterChain).name());

	if (Scale)
	{
		_Transform->SetScale(*Scale);
	}

	if (Rotation)
	{
		_Transform->SetRotation(*Rotation);
	}
	
	_Transform->SetLocation(SpawnLocation);
	
	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"DelasaCenterChain");

	auto _Collision = AddComponent<Engine::Collision>
						(Device, Engine::CollisionTag::Decorator, _Transform,
							typeid(DelasaCenterChain).name());

	_Collision->RenderObjectTransform = _Transform;
	// 바운딩 박스.
	{
		Vector3  BoundingBoxMin{}, BoundingBoxMax{};
		D3DXComputeBoundingBox(_SkeletonMesh->LocalVertexLocations->data(),
			                   _SkeletonMesh->LocalVertexLocations->size(),
			sizeof(Vector3), &BoundingBoxMin, &BoundingBoxMax);

		_Collision->_Geometric = std::make_unique<Engine::OBB>
									(BoundingBoxMin, BoundingBoxMax);

		static_cast<Engine::OBB* const> (_Collision->_Geometric.get())->MakeDebugCollisionBox				(Device);
	}

	_SkeletonMesh->SetUpCullingInformation(_Collision->_Geometric->LocalSphere,
	   _Transform);
	_SkeletonMesh->bCullingOn = true;

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Take 001";
	_SkeletonMesh->PlayAnimation(_AnimNotify);
	
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
			
		});

	_Collision->RefPushCollisionables().insert(
		{
	       
		});
}

void DelasaCenterChain::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();

	bCapturable = true;

	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>(Device,
		App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"",
		L"Delasa_Center_Chain.fbx", L"DelasaCenterChain",
		Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>(L"DelasaCenterChain", _SkeletonMeshProto);
}

void DelasaCenterChain::Event()&
{
	Super::Event();
}

void DelasaCenterChain::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};

void DelasaCenterChain::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
};

void DelasaCenterChain::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void DelasaCenterChain::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
};

std::function<Engine::Object::SpawnReturnValue(
	const Engine::Object::SpawnParam&)> DelasaCenterChain::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;
	
	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);

	if (SpawnSelectCheck)
	{
		return 	[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, DelasaCenterChain>
				(L"Static", L"DelasaCenterChain_" + std::to_wstring(SpawnID++),
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
};


std::shared_ptr<Engine::Object> DelasaCenterChain::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)>  Clone = *this;
	return std::make_shared<DelasaCenterChain>(Clone);
};

std::optional<Engine::Object::SpawnReturnValue> 
DelasaCenterChain::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	DelasaCenterChain::Initialize(_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue{} };
}
