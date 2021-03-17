#include "..\\stdafx.h"
#include "Monster.h"
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

void Monster::MonsterInitialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&
{
	Super::Initialize();
}

void Monster::MonsterPrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();


}
std::shared_ptr<Engine::Object> Monster::GetCopyShared()&
{
	return std::shared_ptr<Engine::Object>();
}
std::optional<Engine::Object::SpawnReturnValue> 
Monster::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	return std::optional<SpawnReturnValue>();
}

std::function < Engine::Object::SpawnReturnValue
(const Engine::Object::SpawnParam&)>
Monster::PrototypeEdit()&
{
	return std::function<SpawnReturnValue(const SpawnParam&)>();
};

void Monster::Event()&
{
	Super::Event();
	auto _Transform = GetComponent<Engine::Transform>();
	Edit();
}

void Monster::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	CurInvincibilityTime -= DeltaTime;
	FSM(DeltaTime);
}

void Monster::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);

	auto _Transform = GetComponent<Engine::Transform>();
	const Vector3 Location = _Transform->GetLocation();

	if (CurrentCell == nullptr)
	{
		auto& NaviMesh = RefNaviMesh();
		CurrentCell = NaviMesh.GetJumpingCellFromXZLocation({ Location.x , Location.z });
	}

	if (CurrentCell)
	{
		auto bCellResult = CurrentCell->Compare(Location);
		if (bCellResult)
		{
			const Engine::Cell::CompareType _CompareType = bCellResult->_Compare;

			if (_CompareType == Engine::Cell::CompareType::Moving)
			{
				CurrentCell = bCellResult->Target;
				_Transform->RefPhysic().CurrentGroundY = bCellResult->ProjectLocation.y;
			}
			else if (_CompareType == Engine::Cell::CompareType::Stop)
			{
				if (CurrentCell->bEnableJumping)
				{
					CurrentCell = nullptr; 
					_Transform->RefPhysic().CurrentGroundY = -1000.f;
				}
				else
				{
					CurrentCell = bCellResult->Target;
					_Transform->RefPhysic().CurrentGroundY = bCellResult->ProjectLocation.y;
				
					_Transform->SetLocation(
						Vector3{ _Transform->PrevLocation.x ,
						_Transform->GetLocation().y ,
						_Transform->PrevLocation.z });
				/*	_Transform->SetLocation(
						{ bCellResult->ProjectLocation.x , Location.y ,bCellResult->ProjectLocation.z });*/
				}
			}
		}
	}

	if (Location.y < -100.f)
	{
		auto& Control = RefControl();
	/*	FSMControlInformation FSMControlInfo
		{ _Transform  , GetComponent<Engine::SkeletonMesh>(), DeltaTime };*/
		// JumpDownTransition(FSMControlInfo);
	}

	if (Location.y < -900.f)
	{
		Kill();
		// _Transform->SetLocation(_Transform->RefPhysic().GetLastLandLocation());
	}
}

void Monster::Edit()&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::InputFloat("DamageToForceFactor", &DamageToForceFactor);
		ImGui::InputFloat("AirDamageToForceFactor", &AirDamageToForceFactor);
	}
}


void Monster::HitNotify(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);
}

void Monster::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
}

void Monster::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target); 
}

float Monster::TakeDamage(const float Damage)& 
{
	CurInvincibilityTime = ResetInvincibilityTime;
	return _Status.HP -= std::fabsf(Damage);
}

bool Monster::IsAttackRange(const Vector3& TargetLocation)
{
	return FMath::Length(GetComponent<Engine::Transform>()->GetLocation() - TargetLocation) < AttackRange;
}

void Monster::FSM(const float DeltaTime)&
{

}

void Monster::LockingToWardsFromDirection(Vector3 Direction)&
{
	Direction = FMath::Normalize(Direction);
	auto*const _Tranfrom=GetComponent<Engine::Transform>(); 
	Vector3 Rotation  = _Tranfrom->GetRotation();
	Rotation.y = std::atan2f(Direction.x, Direction.z) - FMath::PI;
	_Tranfrom->SetRotation(Rotation);
	
}


bool Monster::CheckTheLandingStatable(const float CurLocationY, const float CurGroundY)&
{
	const float      CorrectionLocationY = (CurLocationY - LandCheckHighRange);
	return (CorrectionLocationY < CurGroundY) && ((CurGroundY - CorrectionLocationY) >= 1.f);
};