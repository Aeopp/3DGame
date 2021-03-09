#include "..\\stdafx.h"
#include "Player.h"
#include "Transform.h"
#include "DynamicMesh.h"
#include <iostream>
#include "DynamicMesh.h"
#include "ThirdPersonCamera.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Management.h"
#include "Vertexs.hpp" 
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"
#include "NormalLayer.h"
#include "PlayerHead.h"
#include "PlayerWeapon.h"
#include "PlayerHair.h"

void Player::Initialize(
	const std::optional<Vector3>& Scale,
	const std::optional<Vector3>& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>(typeid(Player).name());

	if (Scale)
	{
		_Transform->SetScale(*Scale);
	}

	if (Rotation)
	{
		_Transform->SetRotation(*Rotation);
	}
	
	_Transform->SetLocation(SpawnLocation);
	
	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"Player");

	auto _Collision = AddComponent<Engine::Collision>
						(Device, Engine::CollisionTag::Player, _Transform,
							typeid(Player).name());

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

	auto& Control = RefControl();
	FSMControlInformation InitFSMControlInfo{ _Transform  , Control    , _SkeletonMesh ,1.f };
	CombatWaitTransition(InitFSMControlInfo);

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
			Engine::CollisionTag::Enemy ,
			Engine::CollisionTag::EnemyAttack
		});

	_Collision->RefPushCollisionables().insert(
		{
	       Engine::CollisionTag::Enemy
		});

	std::shared_ptr<PlayerWeapon> _PlayerWeapon =
		RefManager().NewObject<Engine::NormalLayer, PlayerWeapon >(L"Static",
			Name + L"_Weapon", Vector3{ 0.5f,0.5f,0.5f }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 });

	auto* PlayerWeaponTransform = _PlayerWeapon->GetComponent<Engine::Transform>();
	PlayerWeaponTransform->AttachBone(&_SkeletonMesh->GetBone("Weapon_Hand_R")->ToRoot);
	PlayerWeaponTransform->AttachTransform(&_Transform->UpdateWorld());

	std::shared_ptr<PlayerHead> _PlayerHead = 
		RefManager().NewObject<Engine::NormalLayer, PlayerHead>(L"Static", Name+L"_Head",
			Vector3{ 1,1,1 },
			Vector3{ 0.071,4.774,3.327}, 
			Vector3{ 14.050,-2.910,-0.623});

	auto* PlayerHeadTransform = _PlayerHead->GetComponent<Engine::Transform>();
	PlayerHeadTransform->AttachBone(&_SkeletonMesh->GetBone("Spine2")->ToRoot);
	PlayerHeadTransform->AttachTransform(&_Transform->UpdateWorld() );

	std::shared_ptr<PlayerHair> _PlayerHair =
		RefManager().NewObject<Engine::NormalLayer, PlayerHair >(L"Static",
			Name + L"_Hair", Vector3{ 1,1,1 },
			Vector3{0.071f,4.769f,3.125f}, 
			Vector3{-137.612,-12.285,7.865f});

	auto* PlayerHairTransform = _PlayerHair->GetComponent<Engine::Transform>();
	PlayerHairTransform->AttachBone(&_SkeletonMesh->GetBone("Spine2")->ToRoot);
	PlayerHairTransform->AttachTransform(&_Transform->UpdateWorld());

	constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

	auto& Manager = RefManager(); 

	if (Engine::Global::bDebugMode==false)
	{
		CurrentTPCamera = Manager.NewObject<Engine::NormalLayer, 
			Engine::ThirdPersonCamera>(L"Static", L"ThirdPersonCamera",
			FMath::PI / 4.f, 0.1f, 15000.f, Aspect).get();

		Engine::ThirdPersonCamera::TargetInformation InitTargetInfo{};
		InitTargetInfo.DistancebetweenTarget = 250.f;
		InitTargetInfo.TargetLocationOffset = { 0,50.f,0.f };
		InitTargetInfo.TargetObject = this;
		InitTargetInfo.ViewDirection = { 0.f,-0.707f,0.707f };
		InitTargetInfo.RotateResponsiveness = 0.01f;
		InitTargetInfo.ZoomInOutScale = 0.1f;

		Manager.FindObject<Engine::NormalLayer, Engine::ThirdPersonCamera>(L"ThirdPersonCamera")->SetUpTarget(InitTargetInfo);
	}
}

void Player::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();
	bCapturable = true;
	this->Device = Device;

	auto _SkeletonMeshProto = std::make_shared<Engine::SkeletonMesh>();

	_SkeletonMeshProto->Load<Vertex::LocationTangentUV2DSkinning>(Device,
		App::ResourcePath / L"Mesh" / L"DynamicMesh" / L"",
		L"PlayerAnimation.fbx", L"Player",
		Engine::RenderInterface::Group::DeferredNoAlpha);

	RefResourceSys().InsertAny<decltype(_SkeletonMeshProto)>(L"Player", _SkeletonMeshProto);
}

void Player::Event()&
{
	Super::Event();

	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
}

void Player::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	FSM(DeltaTime);
};

void Player::FSM(const float DeltaTime)&
{
	auto& Control = RefControl();
	auto _Transform = GetComponent<Engine::Transform>();
	auto _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();

	FSMControlInformation CurrentFSMControlInfo{ _Transform, Control, _SkeletonMesh ,DeltaTime };

	switch (CurrentState)
	{
	case Player::State::Run:
		RunState(CurrentFSMControlInfo);
		break;
	case Player::State::CombatWait:
		CombatWaitState(CurrentFSMControlInfo);
		break;
	case Player::State::RunEnd:
		RunEndState(CurrentFSMControlInfo);
		break;
	case Player::State::Jump:
		JumpState(CurrentFSMControlInfo);
		break;
	case Player::State::JumpDown:
		JumpDownState(CurrentFSMControlInfo);
		break;
	case Player::State::JumpStart:
		JumpStartState(CurrentFSMControlInfo);
		break;
	case Player::State::JumpUp:
		JumpUpState(CurrentFSMControlInfo);
		break;
	case Player::State::JumpLanding:
		JumpLandingState(CurrentFSMControlInfo);
		break;
	case Player::State::BasicCombo01:
		BasicCombo01State(CurrentFSMControlInfo);
		break;
	case Player::State::BasicCombo02: 
		BasicCombo02State(CurrentFSMControlInfo); 
		break;
	case Player::State::BasicCombo03:
		BasicCombo03State(CurrentFSMControlInfo);
		break;
	case Player::State::ComboEx01:
		ComboEx01State(CurrentFSMControlInfo);
		break;
	case Player::State::ComboEx02Start:
		ComboEx02StartState(CurrentFSMControlInfo);
		break;
	case Player::State::ComboEx02Loop:
		ComboEx02LoopState(CurrentFSMControlInfo);
		break;
	case Player::State::ComboEx02End:
		ComboEx02EndState(CurrentFSMControlInfo);
		break;
	case Player::State::Dash:
		DashState(CurrentFSMControlInfo);
		break; 
	case Player::State::DashCombo:
		DashComboState(CurrentFSMControlInfo); 
		break; 
	case Player::State::StandUpRolling:
		StandUpRollingState(CurrentFSMControlInfo); 
		break; 
	case Player::State::StandBigBack:
		StandBigBackState(CurrentFSMControlInfo);
		break;
	case Player::State::StandBigFront:
		StandBigFrontState(CurrentFSMControlInfo);
		break;
	case Player::State::StandBigLeft:
		StandBigLeftState(CurrentFSMControlInfo);
		break;
	case Player::State::StandBigRight:
		StandBigRightState(CurrentFSMControlInfo);
		break;
	case Player::State::AirCombo01:
		AirCombo01State(CurrentFSMControlInfo);
		break;
	case Player::State::AirCombo02:
		AirCombo02State(CurrentFSMControlInfo);
		break;
	case Player::State::AirCombo03:
		AirCombo03State(CurrentFSMControlInfo);
		break;
	case Player::State::AirCombo04:
		AirCombo04State(CurrentFSMControlInfo);
		break;
	case Player::State::AirCombo04Landing:
		AirCombo04LandingState(CurrentFSMControlInfo);
		break;
	default:
		break;
	}
};

void Player::RunState(const FSMControlInformation& FSMControlInfo)&
{
	if (CheckTheJumpableState(FSMControlInfo))
	{
		JumpStartState(FSMControlInfo);
		return;
	}

	if (CheckTheAttackableState(FSMControlInfo))
	{
		BasicCombo01Transition(FSMControlInfo);
		return;
	}

	auto bMoveInfo  = CheckTheMoveableState(FSMControlInfo);

	if (bMoveInfo)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LSHIFT))
		{
 			StandUpRollingTransition(FSMControlInfo , *bMoveInfo);
		}
		else 
		{ 
			MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Run);
		}
	}
	else
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

void Player::CombatWaitState(const FSMControlInformation& FSMControlInfo)&
{
	if (CheckTheJumpableState(FSMControlInfo) )
	{
		JumpStartTransition(FSMControlInfo);
		return;
	}

	if (CheckTheAttackableState(FSMControlInfo))
	{
		BasicCombo01Transition(FSMControlInfo);
		return;
	}

	if (FSMControlInfo._Controller.IsDown(DIK_J))
	{
		StandBigFrontTransition(FSMControlInfo);
	}
	else if(FSMControlInfo._Controller.IsDown(DIK_L))
	{
		StandBigBackTransition(FSMControlInfo);
	}
	else if (FSMControlInfo._Controller.IsDown(DIK_L))
	{
		StandBigLeftTransition(FSMControlInfo);
	}
	else if (FSMControlInfo._Controller.IsDown(DIK_O))
	{
		StandBigRightTransition(FSMControlInfo);
	}

	if (auto bMoveable =CheckTheMoveableState(FSMControlInfo))
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LCONTROL))
		{
			DashTransition(FSMControlInfo, *bMoveable);
		}
		else
		{
			RunTransition(FSMControlInfo);
		}
	}
};

void Player::CombatWaitTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "CombatWait";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::CombatWait;
};

void Player::RunTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "run";

	_AnimNotify.AnimTimeEventCallMapping[0.90f] = [](Engine::SkeletonMesh* const Mesh)
	{

	};
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Run;

	const auto& ViewPlayerInfo = CurrentTPCamera->GetTargetInformation();
	Vector3 ViewDirection = ViewPlayerInfo.CurrentViewDirection;
	ViewDirection.y = 0.0f;
	ViewDirection = FMath::Normalize(ViewDirection);

	auto _Transform = GetComponent<Engine::Transform>();
	Vector3 NewRotation = _Transform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	_Transform->SetRotation(NewRotation);
	CurrentMoveDirection = ViewDirection;
};

void Player::RunEndState(const FSMControlInformation& FSMControlInfo)&
{
	if (CheckTheJumpableState(FSMControlInfo))
	{
		JumpStartTransition(FSMControlInfo);
		return;
	}

	if (CheckTheAttackableState(FSMControlInfo))
	{
		BasicCombo01Transition(FSMControlInfo);
		return;
	}

	const auto& CurAnimNotifyInfo = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotifyInfo.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}

};

void Player::JumpStartState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify =FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	
	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo); 
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);	
	}

	Jumping(FSMControlInfo, JumpForce);


	if (CurAnimNotify.bAnimationEnd)
	{
		JumpUpTransition(FSMControlInfo);
		return;
	}
}

void Player::JumpStartTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpStart";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpStart;


}
void Player::JumpUpState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

	Jumping(FSMControlInfo, JumpForce);

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		AirCombo01Transition(FSMControlInfo) ;
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		JumpTransition(FSMControlInfo);
		return;
	}
}
void Player::JumpUpTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpUp";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpUp;

	
}
void Player::JumpState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

	Jumping(FSMControlInfo, JumpForce);


	if (CurAnimNotify.bAnimationEnd)
	{
		JumpDownTransition(FSMControlInfo);
		return;
	}
};

void Player::JumpTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "jump";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Jump;
};

void Player::JumpDownState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

	Jumping(FSMControlInfo, JumpForce);

	if (CurAnimNotify.bAnimationEnd)
	{
		JumpLandingTransition(FSMControlInfo);
		return;

	}
};

void Player::JumpDownTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpDown";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpDown;
};

void Player::JumpLandingState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

	Jumping(FSMControlInfo, JumpForce);

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
}
void Player::JumpLandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpLanding";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpLanding;
}

void Player::ComboEx02StartState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.ComboEx02);
	}

	if (!FSMControlInfo._Controller.IsPressing(DIK_RIGHTCLICK))
	{
		CombatWaitTransition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		ComboEx02LoopTransition(FSMControlInfo);
		return;
	}
}; 

void Player::ComboEx02StartTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "ComboEx02Start";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::ComboEx02Start;
}; 

void Player::ComboEx02LoopState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.ComboEx02);
	};

	if (CurAnimNotify.bAnimationEnd ||
		!FSMControlInfo._Controller.IsPressing(DIK_RIGHTCLICK) )
	{
		ComboEx02EndTransition(FSMControlInfo);
		return;
	}
}


void Player::ComboEx02LoopTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "ComboEx02Loop";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::ComboEx02Loop;
}; 

void Player::ComboEx02EndState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.ComboEx02);
	};

	if (CurAnimNotify.bAnimationEnd )
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};

void Player::ComboEx02EndTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "ComboEx02End";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::ComboEx02End;
};


void Player::ComboEx01State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_SPACE))
		{
			// 여기서 적을 추적해서 점프.... 
			JumpTransition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitState(FSMControlInfo);
		return;
	}
}

void Player::ComboEx01Transition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "ComboEx01";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::ComboEx01;
}

void Player::BasicCombo01State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
		{
			BasicCombo02Transition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitState(FSMControlInfo );
		return;
	}
}


void Player::AirCombo01State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		AirCombo02Transition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return; 
	}
};
void Player::AirCombo01Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo01";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo01;
};

void Player::AirCombo02State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		AirCombo03Transition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};
void Player::AirCombo02Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo02";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo02;
};
void Player::AirCombo03State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		AirCombo04Transition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};
void Player::AirCombo03Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo03";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo03;
};
void Player::AirCombo04State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		AirCombo04LandingTransition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};
void Player::AirCombo04Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo04";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo04;
};
void Player::AirCombo04LandingState(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};
void Player::AirCombo04LandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo04Landing";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo04Landing;
}

void Player::BasicCombo01Transition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "BasicCombo01";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::BasicCombo01;
}

void Player::BasicCombo02State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
		{
			BasicCombo03Transition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitState(FSMControlInfo);
		return;
	}
}

void Player::BasicCombo02Transition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "BasicCombo02";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::BasicCombo02;
}

void Player::BasicCombo03State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
		{
			ComboEx01Transition(FSMControlInfo);
		}

		if (FSMControlInfo._Controller.IsPressing(DIK_RIGHTCLICK))
		{
			ComboEx02StartTransition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitState(FSMControlInfo);
		return;
	}
}

void Player::BasicCombo03Transition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "BasicCombo03";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::BasicCombo03;
};

void Player::RunEndTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "RunEnd";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::RunEnd;
};

void Player::DashState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CheckTheAttackableState(FSMControlInfo))
	{
		DashComboTransition(FSMControlInfo);
		return;
	}

	FSMControlInfo.MyTransform->Move
		(CurrentMoveDirection, FSMControlInfo.DeltaTime,StateableSpeed.Dash);

	if (CurAnimNotify.bAnimationEnd) 
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

void Player::DashTransition(const FSMControlInformation& FSMControlInfo,
				const Player::MoveControlInformation& MoveControlInfo)&
{
	const 	Vector3 ControlMoveDirection = (
		std::accumulate(
			std::begin(MoveControlInfo.ControlDirections),
			std::end(MoveControlInfo.ControlDirections),
			Vector3{ 0.f,0.f,0.f })
		/ static_cast<float>(MoveControlInfo.ControlDirections.size()));

	Vector3 NewRotation = FSMControlInfo.MyTransform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	FSMControlInfo.MyTransform->SetRotation(NewRotation);

	CurrentMoveDirection = ControlMoveDirection;
	CurrentMoveDirection.y = 0.0f;
	CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection);

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "dash";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Dash;
}

std::optional<Player::MoveControlInformation>   
	Player::CheckTheMoveableState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& ViewPlayerInfo = CurrentTPCamera->GetTargetInformation();
	Vector3 ViewDirection = ViewPlayerInfo.CurrentViewDirection;
	ViewDirection.y = 0.0f;
	ViewDirection = FMath::Normalize(ViewDirection);
	std::vector<Vector3> ControlDirections{};

	if (FSMControlInfo._Controller.IsPressing(DIK_W))
	{
		ControlDirections.emplace_back(ViewDirection);
	}
	if (FSMControlInfo._Controller.IsPressing(DIK_S))
	{
		ControlDirections.emplace_back(-ViewDirection);
	}
	if (FSMControlInfo._Controller.IsPressing(DIK_D))
	{
		ControlDirections.emplace_back(FMath::Normalize(FMath::RotationVecNormal(ViewDirection, { 0,1,0 }, FMath::PI / 2.f)));
	}
	if (FSMControlInfo._Controller.IsPressing(DIK_A))
	{
		ControlDirections.emplace_back(FMath::Normalize(FMath::RotationVecNormal(ViewDirection, { 0,1,0 }, -FMath::PI / 2.f)));
	}

	if (ControlDirections.empty() == false)
	{
		return { Player::MoveControlInformation{ std::move(ControlDirections) } };
	}
	
	return std::nullopt;
}

void Player::StandBigBackTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigBack";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigBack;
}

void Player::Jumping(const FSMControlInformation& FSMControlInfo  , const float Force)&
{
	FSMControlInfo.MyTransform->Move({ 0,1,0 },FSMControlInfo.DeltaTime, JumpForce);
}

void Player::StandBigBackState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify=FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo); 
	}
}

void Player::StandBigFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigFront";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigFront;
}

void Player::StandBigFrontState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

void Player::StandBigLeftTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigLeft";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigLeft;
}

void Player::StandBigLeftState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

void Player::StandBigRightTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigRight";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigRight;
}

void Player::StandBigRightState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

bool Player::CheckTheJumpableState(const FSMControlInformation& FSMControlInfo)&
{
	if (FSMControlInfo._Controller.IsDown(DIK_SPACE))
	{
		return true;
	}

	return false;
};

bool Player::CheckTheAttackableState(const FSMControlInformation& FSMControlInfo)&
{
	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK))
	{
		return true;
	}

	return false;
}
void Player::MoveFromController(const FSMControlInformation& FSMControlInfo,
	const Player::MoveControlInformation& MoveControlInfo,
	const float CurrentStateSpeed)&
{
	const 	Vector3 ControlMoveDirection = (
		std::accumulate(
			std::begin(MoveControlInfo.ControlDirections),
			std::end(MoveControlInfo.ControlDirections),
			Vector3{ 0,0,0 })) / MoveControlInfo.ControlDirections.size();

	auto _Transform = GetComponent<Engine::Transform>();
	CurrentMoveDirection = FMath::Normalize(FMath::Lerp(CurrentMoveDirection, ControlMoveDirection,
		FSMControlInfo.DeltaTime * PlayerMoveDirectionInterpolateAcceleration));
	Vector3 NewRotation = _Transform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	_Transform->SetRotation(NewRotation);

	FSMControlInfo.MyTransform->Move
		(CurrentMoveDirection,FSMControlInfo.DeltaTime, CurrentStateSpeed);
};

void Player::DashComboState(const FSMControlInformation& FSMControlInfo)&
{
	const bool bAttackMotionEnd =
		(FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime() > 0.5f);

	if (CheckTheJumpableState(FSMControlInfo) && bAttackMotionEnd)
	{
		JumpStartState(FSMControlInfo);
		return;
	}

	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
	}
}

void Player::StandUpRollingState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	FSMControlInfo.MyTransform->Move
		(CurrentMoveDirection, FSMControlInfo.DeltaTime, StateableSpeed.Rolling);

	if (CurAnimNotify.bAnimationEnd)
	{
		if (CheckTheMoveableState(FSMControlInfo)  ) 
		{
			RunTransition(FSMControlInfo ); 
		}
		else
		{
			CombatWaitTransition(FSMControlInfo);
		}
	}
}

void Player::StandUpRollingTransition(const FSMControlInformation& FSMControlInfo ,
						              const Player::MoveControlInformation& MoveControlInfo )&
{
	const Vector3 ControlMoveDirection = (
		std::accumulate(
			std::begin(MoveControlInfo.ControlDirections),
			std::end(MoveControlInfo.ControlDirections),
			Vector3{ 0.f,0.f,0.f })
		/ static_cast<float>(MoveControlInfo.ControlDirections.size()));

	Vector3 NewRotation = FSMControlInfo.MyTransform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	FSMControlInfo.MyTransform->SetRotation(NewRotation);

	CurrentMoveDirection = ControlMoveDirection;
	CurrentMoveDirection.y = 0.0f;
	CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection);

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandUpRolling";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandUpRolling;
}

void Player::DashComboTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "DashCombo";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::DashCombo;
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

std::function<Engine::Object::SpawnReturnValue(
	const Engine::Object::SpawnParam&)> Player::PrototypeEdit()&
{
	static uint32 SpawnID = 0u;
	
	static bool SpawnSelectCheck = false;
	ImGui::Checkbox("SpawnSelect", &SpawnSelectCheck);

	if (SpawnSelectCheck)
	{
		return 	[&RefManager = Engine::Management::Instance]
		(const Engine::Object::SpawnParam& SpawnParams)->Engine::Object::SpawnReturnValue
		{
			RefManager->NewObject<Engine::NormalLayer, Player>
				(L"Static", L"Player_" + std::to_wstring(SpawnID++),
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


std::shared_ptr<Engine::Object> Player::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)>  Clone = *this;
	return std::make_shared<Player>(Clone);
};

std::optional<Engine::Object::SpawnReturnValue> 
Player::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	Player::Initialize(_SpawnParam.Scale,
		_SpawnParam.Rotation,
		_SpawnParam.Location);

	return { Engine::Object::SpawnReturnValue{} };
}
