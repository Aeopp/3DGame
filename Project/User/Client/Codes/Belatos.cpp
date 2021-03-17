#include "..\\stdafx.h"
#include "Belatos.h"
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
	case Belatos::State::Air:
		AirState(CurrentFSMControlInfo);
		break;
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
	case Belatos::State::DownBack:
		DownBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::DownFront:
		RTDownFrontState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTChaseSmallBack:
		RTChaseSmallBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTChaseBigBack:
		RTChaseBigBackState(CurrentFSMControlInfo); 
		break;
	case Belatos::State::RTChaseBigFront:
		RTChaseBigFrontState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTChaseBigLeft:
		RTChaseBigLeftState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTChaseBigRight:
		RTChaseBigRightState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownAirFallBack:
		RTDownAirFallBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownAirFallFront:
		RTDownAirFallFrontState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownAirLandingBack:
		RTDownAirLandingBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownAirLandingBackLeft:
		RTDownAirLandingBackLeftState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownBack:
		RTDownBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTDownFront:
		RTDownFrontState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStandAirBigFront:
		RTStandAirBigFrontState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStandAirBigBack:
		RTStandAirBigBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStandAirFall:
		RTStandAirFallState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStandAirLanding:
		RTStandAirLandingState(CurrentFSMControlInfo);
		break;
	case Belatos::State::RTStandAirSmall:
		RTStandAirSmallState(CurrentFSMControlInfo);
		break;
	case Belatos::State::StandUpBack:
		StandUpBackState(CurrentFSMControlInfo);
		break;
	case Belatos::State::StandUpFront:
		StandUpFrontState(CurrentFSMControlInfo);
		break;
	default:
		break;
	};
}


void Belatos::DissolveStart(const float SliceAmountSpeed, const float SliceAmoutStart)&
{
	auto* const _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
	Engine::Mesh::DissolveInfo DissolveInfo;
	DissolveInfo.BurnSize = 0.3f;
	DissolveInfo.EmissionAmount = 2.f;
	DissolveInfo.SliceAmount = SliceAmoutStart;
	DissolveInfo.bBlueBurn = false;
	_SkeletonMesh->_DissolveInfo = DissolveInfo;
	this->SliceAmountSpeed = SliceAmountSpeed;
};

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
		auto _Transform = GetComponent<Engine::Transform>();
		auto& Physic= _Transform->RefPhysic();
		_Transform->ClearVelocity();

		const float ForcePitchRad = _PlayerWeapon->ForcePitchRad;
		const bool bAirAttack = !FMath::AlmostEqual(ForcePitchRad, 0.0f);

		const auto DamageRange = _PlayerWeapon->GetDamage(); 
		const float CurDamage = FMath::Random(DamageRange.first, DamageRange.second);
		float DmgToForce = CurDamage;
		if (bAirAttack)
			DmgToForce *= AirDamageToForceFactor;
		else
			DmgToForce *= DamageToForceFactor;
		const Vector3 PlayerLocation = _PlayerWeapon->GetAttackOwner()->GetComponent<Engine::Transform>()->GetLocation();
		// 피격 당한 몹 위치 <- 플레이어 위치
		const Vector3 DistanceFromPlayer= _Transform->GetLocation() - PlayerLocation;
		// 3D 방향 
		const Vector3 PushDirection = FMath::Normalize(DistanceFromPlayer);
		//// XZ 평면에 투영 방향
		const Vector3 PushDirectionProjectionXZ = FMath::Normalize({ PushDirection.x , 0.0f , PushDirection.z });
		// 피격자를 공중에 띄워올리는 공격일 경우 벡터를 얼마나 회전 시킬 것인가 ? 
		
		// 3D 벡터의 Right 벡터를 구해서 해당 축으로 회전 시킬 것임 .
		const Vector3 PushDirectionRight = FMath::Normalize(FMath::Cross(Vector3{ 0,1,0 }, PushDirectionProjectionXZ));
		// 축으로 회전 시킨 이후에 데미지에 비례한 힘만큼 스칼라곱
		const Vector3 HitVelocity = FMath::RotationVecNormal(PushDirectionProjectionXZ, PushDirectionRight, -ForcePitchRad)  *DmgToForce;
		auto* TpCamera =
			dynamic_cast<Engine::ThirdPersonCamera*>(
				dynamic_cast<Player*>(_PlayerWeapon->GetAttackOwner())->CurrentTPCamera);

		const Vector3 Forward = FMath::Normalize(_Transform->GetForward());
		const Vector3 ForwardProjectionXZ = FMath::Normalize({ Forward.x,0.f,Forward.z });

		const Vector3 Right = FMath::Normalize(_Transform->GetRight());
		const Vector3 RightProjectionXZ = FMath::Normalize({ Right.x, 0.f , Right.z }); 
		// 앞,뒤 판별
		static const float Rad45 = std::cosf(FMath::PI / 4.f);
		const float RightDot = FMath::Dot(RightProjectionXZ, PushDirectionProjectionXZ);
		const float ForwardDot = FMath::Dot(ForwardProjectionXZ, PushDirectionProjectionXZ);

		bBackHit = FMath::IsRange(-1.f  , -Rad45, ForwardDot);
		bFrontHit = FMath::IsRange(Rad45, 1.f, ForwardDot);
		bLeftHit = FMath::IsRange(-1.f, -Rad45,RightDot);
		bRightHit = FMath::IsRange(Rad45, 1.f, RightDot);

		static constexpr float CameraStopTime = 0.21f;
		static constexpr float FloatMin = (std::numeric_limits<float>::min)(); 
		// 카메라의 업데이트를 잠시 정지 . 
		TpCamera->bCameraUpdate = false;
		// 카메라 업데이트 정지가 풀리면 몬스터에게 힘을 가하고 정지를 해제함 . 
		RefTimer().TimerRegist(CameraStopTime, 0.0f,  CameraStopTime+ FloatMin, 
			[TpCamera, HitVelocity, _Transform]() {
			_Transform->AddVelocity(HitVelocity);
			TpCamera->bCameraUpdate = true;
			return true;
			});

		static constexpr float ForceTime = 0.18f;
		// 공중에 띄워지는 공격이 아니라면 일정시간 이후에 가해지던 힘을 초기화 ... 
		static constexpr float ForceClearTime = CameraStopTime + ForceTime;
		if (bAirAttack == false)
		{
			RefTimer().TimerRegist(ForceClearTime, 0.0f, ForceClearTime+FloatMin, [this, TpCamera]()
				{
					this->GetComponent<Engine::Transform>()->ClearVelocity();
					return true;
				});
		}

		TakeDamage(CurDamage);

		if (_Status.HP <= 0.0f)
		{
			const float DieTime = 2.f;
			DissolveStart(1.f  / DieTime, 0.f);
			RefTimer().TimerRegist(2.f, 0.0f, 2.1f, [this]() {
				Kill(); return true;  });
		}

		auto*const _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
		auto& _Manager = RefManager();
		auto _Players = _Manager.FindObjects<Engine::NormalLayer, Player>();
		FSMControlInformation InitFSMControlInfo{ _Players.front().get(),_Transform,_SkeletonMesh,1.f };

		if(bAirAttack==false)
		{
			RTStandTransition(InitFSMControlInfo);
		}
		else
		{
			if (bFrontHit)
			{
				RTChaseBigFrontTransition(InitFSMControlInfo);
			}
			else if (bBackHit)
			{
				RTChaseBigBackTransition(InitFSMControlInfo); 
			}
			else if (bRightHit)
			{
				RTChaseBigRightTransition(InitFSMControlInfo); 
			}
			else if (bLeftHit)
			{
				RTChaseBigLeftTransition(InitFSMControlInfo);
			}
		}
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

	auto* const _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
	if (_SkeletonMesh->_DissolveInfo)
	{
		_SkeletonMesh->_DissolveInfo->SliceAmount += DeltaTime * SliceAmountSpeed;

		if (_SkeletonMesh->_DissolveInfo->SliceAmount > (1.0f + DeltaTime * SliceAmountSpeed))
		{
			_SkeletonMesh->_DissolveInfo = std::nullopt;
		}
	}
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
		WaitTransition(FSMControlInfo);
		return;
	}
}

void Belatos::RTStandTransition(const FSMControlInformation& FSMControlInfo)&
{
	FSMControlInfo.MyTransform->ClearVelocity();
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTStand_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStand;
}
void Belatos::AirState(const FSMControlInformation& FSMControlInfo)&
{

};

void Belatos::AirTransition(const FSMControlInformation& FSMControlInfo)&
{
	FSMControlInfo.MyTransform->ClearVelocity();
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Air_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::Air;
};

void Belatos::DownBackState(const FSMControlInformation& FSMControlInfo)&
{

}

void Belatos::DownBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Down_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::DownBack;
}

void Belatos::DownFrontState(const FSMControlInformation& FSMControlInfo)&
{

}

void Belatos::DownFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "Down_F_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::DownFront;
}

void Belatos::RTChaseBigBackState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTChaseBigBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTChase_Big_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTChaseBigBack;
}

void Belatos::RTChaseBigFrontState(const FSMControlInformation& FSMControlInfo)&
{
	auto& Physic = FSMControlInfo.MyTransform->RefPhysic();
	if (Physic.Velocity.y < 0.0f)
	{
		RTDownAirFallFrontTransition(FSMControlInfo);
	}
}

void Belatos::RTChaseBigFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTChase_Big_F_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTChaseBigFront;
}

void Belatos::RTChaseBigLeftState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTChaseBigLeftTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTChase_Big_L_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTChaseBigLeft;
	
}

void Belatos::RTChaseBigRightState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTChaseBigRightTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTChase_Big_R_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTChaseBigRight;

}

void Belatos::RTDownAirFallBackState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTDownAirFallBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTDown_Air_Fall_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownAirFallBack;

}

void Belatos::RTDownAirFallFrontState(const FSMControlInformation& FSMControlInfo)&
{
	const float CurLocationY = FSMControlInfo.MyTransform->GetLocation().y;
	auto& Physic = FSMControlInfo.MyTransform->RefPhysic();

	if (CheckTheLandingStatable(CurLocationY, Physic.CurrentGroundY))
	{
		RTDownAirLandingBackTransition(FSMControlInfo);

		if (bFrontHit)
		{

		}
		else if (bBackHit)
		{

		}
	}
}

void Belatos::RTDownAirFallFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTDown_Air_Fall_F_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownAirFallFront;
}

void Belatos::RTDownAirLandingBackState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify =FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		FSMControlInfo.MyTransform->ClearVelocity();
		StandUpBackTransition(FSMControlInfo);
		return;
	}
}

void Belatos::RTDownAirLandingBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTDown_Air_Landing_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownAirLandingBack;

}

void Belatos::RTDownAirLandingBackLeftState(const FSMControlInformation& FSMControlInfo)&
{

}

void Belatos::RTDownAirLandingBackLeftTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTDown_Air_Landing_B_L_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownAirLandingBackLeft;

}

void Belatos::RTDownBackState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTDownBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTDown_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownBack;


}

void Belatos::RTDownFrontState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTDownFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTDown_F_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTDownFront;


}

void Belatos::RTStandAirBigFrontState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTStandAirBigFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTStand_Air_Big_F_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStandAirBigFront;


}

void Belatos::RTStandAirBigBackState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTStandAirBigBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTStand_Air_Big_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStandAirBigBack;


}

void Belatos::RTStandAirFallState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTStandAirFallTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTStand_Air_Fall_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStandAirFall;

}

void Belatos::RTStandAirLandingState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::RTStandAirLandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RTStand_Air_Landing_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStandAirLanding;
}

void Belatos::RTStandAirSmallState(const FSMControlInformation& FSMControlInfo)&
{

}

void Belatos::RTStandAirSmallTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTStand_Air_Small_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTStandAirSmall;
}

void Belatos::StandUpBackState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify=FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		WaitTransition(FSMControlInfo);
		return;
	}
}

void Belatos::StandUpBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandUp_B_Belatos_Twohadnedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::StandUpBack;

}

void Belatos::StandUpFrontState(const FSMControlInformation& FSMControlInfo)&
{
}

void Belatos::StandUpFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandUp_F_Belatos_Twohadnedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::StandUpFront;

}

void Belatos::RTChaseSmallBackState(const FSMControlInformation& FSMControlInfo)&
{
	auto& Physic = FSMControlInfo.MyTransform->RefPhysic();

	if (Physic.Velocity.y < 0.0f)
	{
		// 여기서 애니메이션 전환.
	}
}

void Belatos::RTChaseSmallBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "RTChase_Small_B_Belatos_Twohandedsword";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = State::RTChaseSmallBack;
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
	ResetInvincibilityTime = 0.05f;
	_Status.HP = 100000.f;
	DamageToForceFactor = 0.1f;
	AirDamageToForceFactor = 0.05f;
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

	DissolveStart(1.f / 3.f, 0.0f);
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

