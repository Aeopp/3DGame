#include "..\\stdafx.h"
#include "Player.h"
#include "Transform.h"
#include "DynamicMesh.h"
#include <iostream>
#include "Management.h"
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

const float TestLandingCheck = 22.7667724609375f;

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
	Engine::Transform::PhysicInformation InitPhysic;
	InitPhysic.Gravity = 220.f;
	_Transform->EnablePhysic(InitPhysic );
	auto _SkeletonMesh = AddComponent<Engine::SkeletonMesh>(L"Player");

	auto _Collision =    AddComponent<Engine::Collision>
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
			Name + L"_Weapon", Vector3{ 1.f,1.f,1.f }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 });
	
	WeaponPut();

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
		bControl = true;

		CurrentTPCamera = Manager.NewObject<Engine::NormalLayer, 
			Engine::ThirdPersonCamera>(L"Static", L"ThirdPersonCamera",
			FMath::PI / 4.f, 0.1f, 777.f, Aspect).get();

		Engine::ThirdPersonCamera::TargetInformation InitTargetInfo{};
		InitTargetInfo.DistancebetweenTarget = 142.8f;
		InitTargetInfo.TargetLocationOffset = { 0,10.f,0.f };
		InitTargetInfo.TargetObject = this;
		InitTargetInfo.ViewDirection = { -0.0120002348f,-0.9999893427f,-0.00832065567f};
		InitTargetInfo.RotateResponsiveness = 0.01f;
		InitTargetInfo.ZoomInOutScale = 0.1f;
		InitTargetInfo.MaxDistancebetweenTarget = 150.f ;

		Manager.FindObject<Engine::NormalLayer, Engine::ThirdPersonCamera>(L"ThirdPersonCamera")->SetUpTarget(InitTargetInfo);
	}
};

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
	Edit();
	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();

	auto& _Control =RefControl();
	if (_Control.IsDown(DIK_O))
	{
		bControl = !bControl;
	}
}

void Player::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	FSM(DeltaTime);



	auto _Transform       = GetComponent<Engine::Transform>();
	const Vector3 Location = _Transform->GetLocation();
	if (Location.y <= TestLandingCheck)
	{
		_Transform->Landing(TestLandingCheck);
	}

	auto& _RefPhysic = _Transform->RefPhysic();


	auto& Control = RefControl();
	if (Control.IsDown(DIK_Z))
	{
		_Transform->Move({ 0,1,0 }, DeltaTime, StateableSpeed.Run);
	}

	if (Control.IsDown(DIK_X))
	{
		_Transform->Move({ 0,-1,0 }, DeltaTime, StateableSpeed.Run);
	}
}

void Player::Edit()&
{
	if (Engine::Global::bDebugMode)
	{
		auto _Transform = GetComponent<Engine::Transform>();
		auto& _Physic = _Transform->RefPhysic();

		ImGui::SliderFloat("JumpInitSpeedY", &JumpInitVelocity.y, 0.f, 300.f);
		ImGui::SliderFloat("Gravity", &_Physic.Gravity, 0.f, 300.f);
		ImGui::SliderFloat("InTheAirSpeed", &StateableSpeed.InTheAirSpeed, 0.f , 100.f);
		ImGui::SliderFloat("DashInitSpeed", &StateableSpeed.Dash, 0.0f, 100.f);
		ImGui::SliderFloat("RollingInitSpeed", &StateableSpeed.Rolling, 0.0f, 100.f);
	}
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
	case Player::State::LeafAttackReady:
		LeafAttackReadyState(CurrentFSMControlInfo);
		break; 
	case Player::State::LeafAttackStart:
		LeafAttackStartState(CurrentFSMControlInfo);
		break;
	case Player::State::LeafAttackUp:
		LeafAttackUpState(CurrentFSMControlInfo);
		break;
	case Player::State::LeafAttackDown:
		LeafAttackDownState(CurrentFSMControlInfo);
		break;
	case Player::State::LeafAttackLanding:
		LeafAttackLandingState(CurrentFSMControlInfo);
		break; 
	default:
		break;
	}
};

void Player::RunState(const FSMControlInformation& FSMControlInfo)&
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

	auto bMoveInfo  = CheckTheMoveableState(FSMControlInfo);

	if (bMoveInfo)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LSHIFT)&& bControl)
		{
 			StandUpRollingTransition(FSMControlInfo , *bMoveInfo);
		}
		else if (FSMControlInfo._Controller.IsDown(DIK_LCONTROL) && bControl)
		{
			DashTransition(FSMControlInfo, *bMoveInfo);
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

	if (CheckTheLeafAttackableState(FSMControlInfo))
	{
		LeafAttackReadyTransition(FSMControlInfo);
		return; 
	}

	if (FSMControlInfo._Controller.IsDown(DIK_J) && bControl)
	{
		StandBigFrontTransition(FSMControlInfo);
	}
	else if(FSMControlInfo._Controller.IsDown(DIK_L) && bControl)
	{
		StandBigBackTransition(FSMControlInfo);
	}
	else if (FSMControlInfo._Controller.IsDown(DIK_L) && bControl)
	{
		StandBigLeftTransition(FSMControlInfo);
	}
	else if (FSMControlInfo._Controller.IsDown(DIK_O) && bControl)
	{
		StandBigRightTransition(FSMControlInfo);
	}

	if (auto bMoveable =CheckTheMoveableState(FSMControlInfo))
	{
        RunTransition(FSMControlInfo);
	}
};

void Player::CombatWaitTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "CombatWait";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::CombatWait;
	WeaponPut();
};

void Player::RunTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();
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

	if (CurAnimNotify.bAnimationEnd)
	{
		JumpUpTransition(FSMControlInfo);
		return;
	}
}

void Player::JumpStartTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpStart";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpStart;

	Vector3 CurLocation = FSMControlInfo.MyTransform->GetLocation();
	CurLocation += (JumpInitVelocity  * FSMControlInfo.DeltaTime);
	FSMControlInfo.MyTransform->SetLocation(CurLocation);
	FSMControlInfo.MyTransform->AddVelocity(JumpInitVelocity);
}
void Player::JumpUpState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo01Transition(FSMControlInfo) ;
		return;
	}

	bool bNextChangeNextJumpMotion = false;

	const auto& _RefPhysic     =  FSMControlInfo.MyTransform->RefPhysic();
	bNextChangeNextJumpMotion |= _RefPhysic.Velocity.y <= 0.0f;
	bNextChangeNextJumpMotion |=  IsSpeedInTheAir(_RefPhysic.Velocity.y);

	if (bNextChangeNextJumpMotion)
	{
		JumpTransition(FSMControlInfo);
		return;
	}
}
void Player::JumpUpTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
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

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo01Transition(FSMControlInfo);
		return;
	}

	bool bNextJumpMotionChange = false;
	const auto& _RefPhysic  = FSMControlInfo.MyTransform->RefPhysic();
	bNextJumpMotionChange |= _RefPhysic.Velocity.y <= 0.0f; 

	if (bNextJumpMotionChange)
	{
		JumpDownTransition(FSMControlInfo);
		return;
	}
};

void Player::JumpTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
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

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo01Transition(FSMControlInfo);
		return;
	}

	auto _Transform = FSMControlInfo.MyTransform;
	auto& CurLocation= _Transform->GetLocation();

	bool bNextJumpMotionChange = false;
	bNextJumpMotionChange |= CurLocation.y <= TestLandingCheck;

	if (bNextJumpMotionChange)
	{
		JumpLandingTransition(FSMControlInfo);
		_Transform->Landing(TestLandingCheck);
		return;
	}

};

void Player::JumpDownTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "JumpDown";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::JumpDown;
};

void Player::JumpLandingState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
}
void Player::JumpLandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

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
		if (FSMControlInfo._Controller.IsDown(DIK_SPACE) && bControl)
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
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
		{
			BasicCombo02Transition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
}


void Player::AirCombo01State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
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
	WeaponHand();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo01";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo01;
};

void Player::AirCombo02State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
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

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
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

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
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
	WeaponHand();
}

void Player::BasicCombo02State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify  = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
		{
			BasicCombo03Transition(FSMControlInfo);
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
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
	WeaponHand();
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
		if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
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



	if (CurAnimNotify.bAnimationEnd) 
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}

	
}

void Player::DashTransition(const FSMControlInformation& FSMControlInfo,
				const Player::MoveControlInformation& MoveControlInfo)&
{
	WeaponPut();

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
	_AnimNotify.AnimTimeEventCallMapping[0.4f] = [this](Engine::SkeletonMesh* const SkMesh)
	{
		GetComponent<Engine::Transform>()->AddVelocity(-CurrentMoveDirection * StateableSpeed.Dash);
	};
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Dash;

	


	FSMControlInfo.MyTransform->AddVelocity(CurrentMoveDirection * StateableSpeed.Dash);
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
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigBack";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigBack;
}

//void Player::Jumping(const FSMControlInformation& FSMControlInfo  , const float Force)&
//{
//	// FSMControlInfo.MyTransform->Move({ 0,1,0 },FSMControlInfo.DeltaTime, JumpForce);
//}

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
	WeaponPut();

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
	WeaponPut();

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
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigRight";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigRight;
}

void Player::LeafAttackReadyState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	// TODO :: 여기서 움직일 수 있으며 마법진 (?) 을 바닥에 그려주는 인터페이스를 제공하면 좋을듯 !
	if (auto bIsMove = CheckTheMoveableState(FSMControlInfo);bIsMove )
	{
		MoveFromController(FSMControlInfo  , *bIsMove , StateableSpeed.LeafReady);
	}

	if (FSMControlInfo._Controller.IsUp(DIK_R))
	{
		// 여기서 마법진 위치로 계산해서 날아가게 하는 처리를 하면 좋을듯 ! 
		LeafAttackStartTransition(FSMControlInfo);
		const float LeafStartTestForce = 33.3f;
		FSMControlInfo.MyTransform->AddVelocity(CurrentMoveDirection * LeafStartTestForce);
	}
}


void Player::LeafAttackReadyTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponHand();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "LeafAttackReady";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackReady;
}

void Player::LeafAttackStartState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		LeafAttackStartTransition(FSMControlInfo);
	}
}

void Player::LeafAttackStartTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "LeafAttackStart";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackStart;
}

void Player::LeafAttackUpState(const FSMControlInformation& FSMControlInfo)&
{
	// 여기서 중력 가속도가  점프 속도를 초과 해서 음수가나올 경우 애니메이션을 전환해야한다 .

	const auto& _Physic =  FSMControlInfo.MyTransform->RefPhysic();
	if (_Physic.Velocity.y < 0.0f)
	{
		LeafAttackDownTransition(FSMControlInfo);
	}
}

void Player::LeafAttackUpTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "LeafAttackUp";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackUp;
}

void Player::LeafAttackDownState(const FSMControlInformation& FSMControlInfo)&
{
	// 여기서 땅에 닿기 이전까지는 해당 모션을 계속 유지해야 한다.
	const Vector3 CurLocation   = FSMControlInfo.MyTransform->GetLocation();

	if ( CurLocation.y < TestLandingCheck   )  
	{
		LeafAttackLandingTransition(FSMControlInfo);
	}
}

void Player::LeafAttackDownTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "LeafAttackDown";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackDown;
}

void Player::LeafAttackLandingState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo); 
	}
}

void Player::LeafAttackLandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "LeafAttackLanding";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackLanding;
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
	if (FSMControlInfo._Controller.IsDown(DIK_SPACE) && bControl)
	{
		return true;
	}

	return false;
};

bool Player::CheckTheAttackableState(const FSMControlInformation& FSMControlInfo)&
{
	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		return true;
	}

	return false;
};

bool Player::CheckTheLeafAttackableState(const FSMControlInformation& FSMControlInfo)&
{
	if (FSMControlInfo._Controller.IsDown(DIK_R))
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
	           (CurrentMoveDirection, FSMControlInfo.DeltaTime, CurrentStateSpeed);
};

bool Player::IsSpeedInTheAir(const float YAxisVelocity)&
{
	if (std::fabsf(YAxisVelocity) < StateableSpeed.InTheAirSpeed)
	{
		return true; 
	}

	return false;
};

// Weapon_Hand_R
// Weapon_Pelvis_R
static void WeaponAttachImplementation(Player* const _Player,
	const std::string& AttachBoneName ,
	const Vector3& OffsetScale,
	const Vector3& OffsetRotation,
	const Vector3& OffsetLocation)
{
	auto& _Manager = RefManager();

	auto Weapon = _Manager.FindObject<Engine::NormalLayer, PlayerWeapon>(_Player->GetName() + L"_Weapon");
	auto _SkeletonMesh = _Player->GetComponent<Engine::SkeletonMesh>();
	auto _Transform = _Player->GetComponent<Engine::Transform>();
	
	if (Weapon)
	{
		auto PlayerWeapon = Weapon->GetComponent<Engine::Transform>();
		PlayerWeapon->AttachBone(&_SkeletonMesh->GetBone(AttachBoneName)->ToRoot);
		PlayerWeapon->AttachTransform(&_Transform->UpdateWorld());
		PlayerWeapon->SetScale(OffsetScale);
		PlayerWeapon->SetRotation(OffsetRotation);
		PlayerWeapon->SetLocation(OffsetLocation);
	}
};

void Player::WeaponPut()&
{
	WeaponAttachImplementation(this, "Weapon_Pelvis_R", 
		{0.90f,0.90f,0.90f },
		{0,-2.583f,0}, 
		{-20.263f,42.553f,-88.813f} );
};

void Player::WeaponHand()&
{
	WeaponAttachImplementation(this, "Weapon_Hand_R" , 
	{1,1,1},
	{0,0,0},
	{0,0,0});
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

	if (CurAnimNotify.bAnimationEnd)
	{
		auto& _RefPhysic = FSMControlInfo.MyTransform->RefPhysic();
		_RefPhysic.Velocity.x = 0.0f;
		_RefPhysic.Velocity.z = 0.0f;

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
	WeaponPut();

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

	// 스탠드 업 ?? 

	CurrentMoveDirection = ControlMoveDirection;
	CurrentMoveDirection.y = 0.0f;
	CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection);

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandUpRolling";
	_AnimNotify.AnimTimeEventCallMapping[0.3f] = [this](Engine::SkeletonMesh* const SkMesh)
	{
		GetComponent<Engine::Transform>()->AddVelocity(-CurrentMoveDirection * StateableSpeed.Rolling);
	};

	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandUpRolling;
	FSMControlInfo.MyTransform->AddVelocity(CurrentMoveDirection * StateableSpeed.Rolling);
}

void Player::DashComboTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponHand();

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
