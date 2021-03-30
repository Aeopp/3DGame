#include "..\\stdafx.h"
#include "AnimationTester.h"
#include "Transform.h"
#include "Management.h"
#include "ThirdPersonCamera.h"
#include "PlayerWeapon.h"
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
#include "NormalLayer.h"
#include "Player.h"
#include "Timer.h"
#include "Sound.h"
#include "Renderer.h"
#include "ObjectEdit.h"
#include "AttachTarget.h"

void AnimationTester::Initialize(
	const std::optional<Vector3>& Scale,
	const std::optional<Vector3>& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

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

	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"AnimationTester");

	auto _Collision = AddComponent<Engine::Collision>
		(Device, Engine::CollisionTag::Decorator, _Transform,
			typeid(MyType).name());

	_Collision->RenderObjectTransform = _Transform;
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

	_SkeletonMesh->SetUpCullingInformation(_Collision->_Geometric->LocalSphere,
		_Transform);
	_SkeletonMesh->bCullingOn = true;

	//Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	//_AnimNotify.bLoop = true;
	//_AnimNotify.Name = "Take 001";
	//_SkeletonMesh->PlayAnimation(_AnimNotify);

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

	std::shared_ptr<AttachTarget> _AttachTarget =
		RefManager().NewObject<Engine::NormalLayer, AttachTarget >(L"Static",
			Name + L"_AttachTarget", Vector3{ 1,1,1 },
			Vector3{ 0.f,0.f,0.f },
			Vector3{0,0,0 });
	this->_AttackTarget = _AttachTarget.get();

	
}


void AnimationTester::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();

	bCapturable = true;

	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>(Device,
		App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"",
		L"AnimationTester.fbx", L"AnimationTester",
		Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>(L"AnimationTester", _SkeletonMeshProto);
}

void AnimationTester::Event()&
{
	Super::Event();

	if (Engine::Global::bDebugMode)
	{
		if (_AttackTarget)
		{
			ImGui::InputText("Bone Name", &EditBoneNameBuf[0],EditBoneNameBuf.size()) ;

			if (ImGui::Button("Attach"))
			{
				auto _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
				auto _Transform = GetComponent<Engine::Transform>();

				std::string Name = EditBoneNameBuf.data();

				auto* _AttachTargetTransform = _AttackTarget->GetComponent<Engine::Transform>();
				_AttachTargetTransform->AttachBone(&_SkeletonMesh->GetBone(Name)->ToRoot);
				_AttachTargetTransform->AttachTransform(&_Transform->UpdateWorld());
			}
			
		}
		
	}
}

void AnimationTester::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};

void AnimationTester::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
};

void AnimationTester::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void AnimationTester::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
};

std::function<Engine::Object::SpawnReturnValue(
	const Engine::Object::SpawnParam&)> AnimationTester::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;

	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);

	if (SpawnSelectCheck)
	{
		return[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, AnimationTester>
				(L"Static", L"AnimationTester_" + std::to_wstring(SpawnID++),
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


std::shared_ptr<Engine::Object> AnimationTester::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)>  Clone = *this;
	return std::make_shared<AnimationTester>(Clone);
};

std::optional<Engine::Object::SpawnReturnValue>
AnimationTester::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	AnimationTester::Initialize(_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue{} };
}

