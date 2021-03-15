#include "..\\stdafx.h"
#include "Belatos.h"
#include "Transform.h"
#include "Management.h"
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

// Point_GS_Root

void Belatos::FSM(const float DeltaTime)&
{
	Super::FSM(DeltaTime);

	auto& Control = RefControl();
	auto _Transform = GetComponent<Engine::Transform>();
	auto _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
	auto& _Manager =RefManager();
	auto _Players =_Manager.FindObjects<Engine::NormalLayer, Player>();
	FSMControlInformation CurrentFSMControlInfo{ _Players.front().get(),_Transform,  _SkeletonMesh ,DeltaTime };

	switch (CurrentState)
	{
	case Belatos::State::Wait:
		WaitState(CurrentFSMControlInfo);
		break;
	case Belatos::State::Run:
		RunState(CurrentFSMControlInfo); 
		break;
	case Belatos::State::RunEnd:
		RunEndState(CurrentFSMControlInfo); 
		break; 
	case Belatos::State::Skill1st:
		Skill1stState(CurrentFSMControlInfo);
		break;
	case Belatos::State::Skill2nd:
		Skill2ndState(CurrentFSMControlInfo);
		break; 
	case Belatos::State::Respawn:
		RespawnState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStand:
		RTStandState(CurrentFSMControlInfo);
		break;
	default:
		break;
	};
}

void Belatos::Edit()&
{
	Super::Edit();
		// ImGui로 변수 통제 . 
	if (Engine::Global::bDebugMode)
	{
		auto _Transform = GetComponent<Engine::Transform>();
		auto& _Physic = _Transform->RefPhysic();
		ImGui::SliderFloat("LandCheckHighRange", &LandCheckHighRange, 0.f, 30.f);

		/*auto* _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
		ImGui::SliderFloat3("WeaponCollisionMin", WeaponLocalMin, -300.f, +300.f);
		ImGui::SliderFloat3("WeaponCollisionMax", WeaponLocalMax, -300.f, +300.f);
		WeaponHandleBone = _SkeletonMesh->GetBone("Point_GS_Root").get();
		WeaponHandleBone->CollisionGeometric = std::make_unique<Engine::OBB>
		 (Engine::OBB{ WeaponLocalMin,
					WeaponLocalMax });
		auto& WeaponHandleOBB = dynamic_cast<Engine::OBB&>(*WeaponHandleBone->CollisionGeometric);
		WeaponHandleOBB.MakeDebugCollisionBox(Device);
		WeaponHandleOBB.Update(_Transform->UpdateWorld());*/
	}
}


void Belatos::HitNotify(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
}

void Belatos::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);

	if (auto _PlayerWeapon = dynamic_cast<PlayerWeapon* const>(Target);
		_PlayerWeapon && !IsInvincibility())
	{
		const Vector3 PushDirNormalize  = FMath::Normalize(PushDir);
		auto _Transofrm = GetComponent<Engine::Transform>();
		const float Dmg = _PlayerWeapon->GetDamage(); 
		const float DmgToForce = Dmg * 0.001f;
		_Transofrm->AddVelocity(PushDirNormalize * DmgToForce); 
		TakeDamage(Dmg);

		auto*const _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
		auto& _Manager = RefManager();
		auto _Players = _Manager.FindObjects<Engine::NormalLayer, Player>();
		FSMControlInformation InitFSMControlInfo{ _Players.front().get(),_Transofrm,_SkeletonMesh,1.f };
		RTStandTransition(InitFSMControlInfo);
	}
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
};

void Belatos::WaitTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Wait_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Belatos::State::Wait;
};

void Belatos::WaitState(const FSMControlInformation& FSMControlInfo)&
{
	RunTransition(FSMControlInfo);
}

void Belatos::RunState(const FSMControlInformation& FSMControlInfo)&
{
	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	auto _PlayerTransform = FSMControlInfo._Player->GetComponent<Engine::Transform>();
	const Vector3 PlayerLocation=_PlayerTransform->GetLocation();
	const Vector3 ToPlayer = PlayerLocation - Location;
	const Vector3 ToPlayerDir = FMath::Normalize(ToPlayer);
	LockingToWardsFromDirection(ToPlayerDir);
	FSMControlInfo.MyTransform->Move(ToPlayerDir* StateableSpeed.Run, FSMControlInfo.DeltaTime);

	if (IsAttackRange(PlayerLocation))
	{
		RunEndTransition(FSMControlInfo);
		return;
	}
}

void Belatos::RunTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Run_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::Run;
}

void Belatos::RunEndState(const FSMControlInformation& FSMControlInfo)&
{
	auto _PlayerTransform = FSMControlInfo._Player->GetComponent<Engine::Transform>();
	const Vector3 PlayerLocation  =_PlayerTransform->GetLocation();

	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		if (IsAttackRange(PlayerLocation))
		{
			FMath::Random(0u, 1u) ? Skill1stTransition(FSMControlInfo) : Skill2ndTransition(FSMControlInfo);
		}
		else
		{
			RunTransition(FSMControlInfo);
		}
		return;
	}
}

void Belatos::RunEndTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "Run_End_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RunEnd;
}

void Belatos::Skill1stState(const FSMControlInformation& FSMControlInfo)&
{
	const auto&   CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const Vector3 PlayerLocation = FSMControlInfo._Player->GetComponent<Engine::Transform>()->GetLocation();

	const Vector3 Forward = FSMControlInfo.MyTransform->GetForward();
	const float CurAnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (CurAnimNotify.bAnimationEnd)
	{
		if (IsAttackRange(PlayerLocation))
		{
			FMath::Random(0u, 1u) ? Skill1stTransition(FSMControlInfo) : Skill2ndTransition(FSMControlInfo);
		}
		else
		{
			WaitTransition(FSMControlInfo);
		}
		return;
	}

	if (CurAnimTime >= 0.27f && CurAnimTime <= 0.35f)
	{
		WeaponAttackCollisionSweep(FSMControlInfo);
	}	
}

void Belatos::Skill1stTransition(const FSMControlInformation& FSMControlInfo)&
{ 
	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	auto _PlayerTransform = FSMControlInfo._Player->GetComponent<Engine::Transform>();
	const Vector3 PlayerLocation = _PlayerTransform->GetLocation();
	const Vector3 ToPlayer = PlayerLocation - Location;
	const Vector3 ToPlayerDir = FMath::Normalize(ToPlayer);
	LockingToWardsFromDirection(ToPlayerDir);
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "Skill_01_1_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::Skill1st;
}

void Belatos::Skill2ndState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const Vector3 PlayerLocation = FSMControlInfo._Player->GetComponent<Engine::Transform>()->GetLocation();

	if (CurAnimNotify.bAnimationEnd)
	{
		if (IsAttackRange(PlayerLocation))
		{
			FMath::Random(0u, 1u) ? Skill1stTransition(FSMControlInfo) : Skill2ndTransition(FSMControlInfo);
		}
		else
		{
			WaitTransition(FSMControlInfo);
		}
		return;
	}

	const float CurAnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (CurAnimTime >= 0.22f && CurAnimTime <= 0.33f)
	{
		WeaponAttackCollisionSweep(FSMControlInfo);
	}
}

void Belatos::Skill2ndTransition(const FSMControlInformation& FSMControlInfo)&
{
	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	auto _PlayerTransform = FSMControlInfo._Player->GetComponent<Engine::Transform>();
	const Vector3 PlayerLocation = _PlayerTransform->GetLocation();
	const Vector3 ToPlayer = PlayerLocation - Location;
	const Vector3 ToPlayerDir = FMath::Normalize(ToPlayer);
	LockingToWardsFromDirection(ToPlayerDir);
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "Skill_01_2_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::Skill2nd;
}

void Belatos::RespawnState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		WaitTransition(FSMControlInfo);
		return;
	}
}

void Belatos::RespawnTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "Respawn_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::Respawn;
}

void Belatos::RTStandState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		bInvincibility = false;
		WaitTransition(FSMControlInfo);
		return;
	}
}

void Belatos::RTStandTransition(const FSMControlInformation& FSMControlInfo)&
{
	bInvincibility = true;
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTStand_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStand;
}


void Belatos::WeaponAttackCollisionSweep(const FSMControlInformation& FSMControlInfo)&
{
	WeaponHandleBone->CollisionGeometric->Update(WeaponHandleBone->ToRoot * FSMControlInfo.MyTransform->UpdateWorld());
	auto* const _PlayerCollision = FSMControlInfo._Player->GetComponent<Engine::Collision>();
	auto CollisionResult = _PlayerCollision->_Geometric->IsCollision(WeaponHandleBone->CollisionGeometric.get());
	if (CollisionResult)
	{
		const float CrossAreaScale = CollisionResult->first;
		const Vector3 PushDirection = CollisionResult->second;
		_PlayerCollision->Owner->Hit(this, PushDirection, CrossAreaScale);
	}
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

	AttackRange = 25.f;

	WeaponLocalMin = {-26.549f  , 31.858f , -17.213f};
	WeaponLocalMax = {109.091f , 223.141f , 56.557f};
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

	_Collision->RefCollisionables().insert({
		Engine::CollisionTag::Enemy,
		Engine::CollisionTag::Player,
		Engine::CollisionTag::PlayerAttack
		});

	_Collision->RefPushCollisionables().insert({
		Engine::CollisionTag::Enemy,
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

	WeaponHandleBone = _SkeletonMesh->GetBone("Point_GS_Root").get();
	WeaponHandleBone->CollisionGeometric = std::make_unique<Engine::OBB>
		(Engine::OBB ( WeaponLocalMin,
					   WeaponLocalMax  ));
	auto& WeaponHandleOBB =dynamic_cast<Engine::OBB&>(*WeaponHandleBone->CollisionGeometric);
	WeaponHandleOBB.MakeDebugCollisionBox(Device);
	WeaponHandleOBB.Update(WeaponHandleBone->ToRoot*_Transform->UpdateWorld());

	auto& _Manager = RefManager();
	auto _Players = _Manager.FindObjects<Engine::NormalLayer, Player>();
	FSMControlInformation InitFSMControlInfo{ _Players.front().get(),_Transform,_SkeletonMesh,1.f };
	WaitTransition(InitFSMControlInfo);
};

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

