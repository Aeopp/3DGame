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
#include "NavigationMesh.h"






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

	if (Engine::Global::bDebugMode == false)
	{
		bControl = true;

		CurrentTPCamera = Manager.NewObject<Engine::NormalLayer,
			Engine::ThirdPersonCamera>(L"Static", L"ThirdPersonCamera",
				FMath::PI / 4.f, 0.1f, 777.f, Aspect).get();

		Engine::ThirdPersonCamera::TargetInformation InitTargetInfo{};
		InitTargetInfo.DistancebetweenTarget = 142.8f;
		InitTargetInfo.TargetLocationOffset = { 0,10.f,0.f };
		InitTargetInfo.TargetObject = this;
		InitTargetInfo.ViewDirection = { -0.0120002348f,-0.9999893427f,-0.00832065567f };
		InitTargetInfo.RotateResponsiveness = 0.01f;
		InitTargetInfo.ZoomInOutScale = 0.1f;
		InitTargetInfo.MaxDistancebetweenTarget = 150.f;

		Manager.FindObject<Engine::NormalLayer, Engine::ThirdPersonCamera>(L"ThirdPersonCamera")->SetUpTarget(InitTargetInfo);
	}

	auto&  _NaviMesh  = RefNaviMesh();
	Vector2 SpawnLocation2D = {SpawnLocation.x, SpawnLocation.z};


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
		InitPhysic.CurrentGroundY = bCellResult->Y;
	};

	_Transform->EnablePhysic(InitPhysic);

	PrevLocation = _Transform->GetLocation();
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
	auto _Transform = GetComponent<Engine::Transform>();
	PrevLocation = _Transform->GetLocation();
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

	auto _Transform = GetComponent<Engine::Transform>();
	const Vector3 Location = _Transform->GetLocation();

	auto& _RefPhysic = _Transform->RefPhysic();
	auto& Control = RefControl();

	// 테스트 
	/*if (Control.IsDown(DIK_DELETE))
	{
		_LeafAttackInfo.Reset(Location, {-423.641f,22.257f,431.255f}, 300.f, 7.f, 0.0f);
		_RefPhysic.bGravityEnable = false;
	};

	auto bMoveLocation = _LeafAttackInfo.Move(DeltaTime);

	if (bMoveLocation)
	{
		_Transform->SetLocation(*bMoveLocation);
	}
	else
	{
		_RefPhysic.bGravityEnable = true;
	}*/

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

		ImGui::SliderFloat("JumpVelocityY", &StateableSpeed.JumpVelocity.y, 0.f, 300.f);
		ImGui::SliderFloat("Gravity", &_Physic.Gravity, 0.f, 300.f);
		ImGui::SliderFloat("InTheAirSpeed", &StateableSpeed.InTheAirSpeed, 0.f , 100.f);
		ImGui::SliderFloat("DashInitSpeed", &StateableSpeed.Dash, 0.0f, 100.f);
		ImGui::SliderFloat("RollingInitSpeed", &StateableSpeed.Rolling, 0.0f, 100.f);

		ImGui::SliderFloat("AirCombo01VelocityY", &StateableSpeed.AirCombo01Velocity.y, 0.f, 300.f);
		ImGui::SliderFloat("AirCombo02VelocityY", &StateableSpeed.AirCombo02Velocity.y, 0.f, 300.f);
		ImGui::SliderFloat("AirCombo03VelocityY", &StateableSpeed.AirCombo03Velocity.y, 0.f, 300.f);
		ImGui::SliderFloat("AirCombo04VelocityY", &StateableSpeed.AirCombo04Velocity.y, -300.f, 0.f);

		ImGui::SliderFloat("LandCheckHighRange", &LandCheckHighRange, 0.f, 30.f);
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
        RunTransition(FSMControlInfo ,bMoveable);
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

void Player::RunTransition(const FSMControlInformation& FSMControlInfo,
						   const std::optional<Player::MoveControlInformation>&
							MoveInfo)&
{
	WeaponPut();
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "run";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Run;

	if (MoveInfo.has_value())
	{
		CurrentMoveDirection =std::accumulate(
			std::begin(MoveInfo->ControlDirections),
			std::end(MoveInfo->ControlDirections), 
			Vector3{ 0, 0, 0 }) 
						/
			static_cast<float> (MoveInfo->ControlDirections.size()); 

		CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection); 
	}

	auto _Transform = GetComponent<Engine::Transform>();
	Vector3 NewRotation = _Transform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	_Transform->SetRotation(NewRotation);
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
	CurLocation += (StateableSpeed.JumpVelocity * FSMControlInfo.DeltaTime);
	FSMControlInfo.MyTransform->SetLocation(CurLocation);
	FSMControlInfo.MyTransform->AddVelocity(StateableSpeed.JumpVelocity);
};

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

	const auto& Physic = _Transform->RefPhysic();

	bool bNextJumpMotionChange = false;
	bNextJumpMotionChange |= (CurLocation.y - LandCheckHighRange) < Physic.CurrentGroundY;

	if (bNextJumpMotionChange)
	{
		JumpLandingTransition(FSMControlInfo);
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

	if (AnimTimeNormal < 0.7f)
	{
		if (CheckTheJumpableState(FSMControlInfo))
		{
			JumpStartTransition(FSMControlInfo);
			return;
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

	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();

	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | ((Location.y - LandCheckHighRange) < Physic.CurrentGroundY));

	if (bLandingChange)
	{
		JumpLandingTransition(FSMControlInfo);
		return;
	}

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo02Transition(FSMControlInfo);
		return;
	}


	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
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
	FSMControlInfo.MyTransform->ClearVelocity();

	FSMControlInfo.MyTransform->AddVelocity(StateableSpeed.AirCombo01Velocity);
};

void Player::AirCombo02State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();


	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();

	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | ((Location.y - LandCheckHighRange) < Physic.CurrentGroundY));

	if (bLandingChange)
	{
		JumpLandingTransition(FSMControlInfo);
		return;
	}

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo03Transition(FSMControlInfo);
		return;
	}


	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}


};
void Player::AirCombo02Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo02";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo02;
	FSMControlInfo.MyTransform->ClearVelocity();
	FSMControlInfo.MyTransform->AddVelocity(StateableSpeed.AirCombo02Velocity);
};
void Player::AirCombo03State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();
	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | ((Location.y - LandCheckHighRange) < Physic.CurrentGroundY));
	if (bLandingChange)
	{
		JumpLandingTransition(FSMControlInfo);
		return;
	}

	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl)
	{
		AirCombo04Transition(FSMControlInfo);
		return;
	}


	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Jump);
	}

};
void Player::AirCombo03Transition(const FSMControlInformation& FSMControlInfo)& 
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "AirCombo03";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::AirCombo03;
	FSMControlInfo.MyTransform->ClearVelocity();

	FSMControlInfo.MyTransform->AddVelocity(StateableSpeed.AirCombo03Velocity);
};
void Player::AirCombo04State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	
	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto & Physic  = FSMControlInfo.MyTransform->RefPhysic();

	bool bLandingChange = (CurAnimNotify.bAnimationEnd | ((Location.y - LandCheckHighRange) < Physic.CurrentGroundY));

	if (bLandingChange)
	{
		AirCombo04LandingTransition(FSMControlInfo);
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
	FSMControlInfo.MyTransform->ClearVelocity();
	FSMControlInfo.MyTransform->AddVelocity(StateableSpeed.AirCombo04Velocity);
};
void Player::AirCombo04LandingState(const FSMControlInformation& FSMControlInfo)& 
{
	const float AnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (AnimTime > 0.97f)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
};
void Player::AirCombo04LandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "AirCombo04Landing";
	/*_AnimNotify.AnimTimeEventCallMapping[0.97f] = 
		[this, FSMControlInfo](Engine::SkeletonMesh*const  SkMesh)
	{
		this->(FSMControlInfo);
	};*/
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
			return;
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
		FSMControlInfo.MyTransform->ClearVelocity();
		return;
	}

	if (CurAnimNotify.bAnimationEnd) 
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
	const float CurNormalzieAnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (CurNormalzieAnimTime > 0.4f )
	{
		if (auto bMoveable = CheckTheMoveableState(FSMControlInfo);
			bMoveable)
		{
			RunTransition(FSMControlInfo,std::nullopt);
			return; 
		}
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
	
	CurrentMoveDirection = ControlMoveDirection;
	CurrentMoveDirection.y = 0.0f;
	DashDirection  = CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection);
	Vector3 NewRotation = FSMControlInfo.MyTransform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	FSMControlInfo.MyTransform->SetRotation(NewRotation);

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "dash";
	_AnimNotify.AnimTimeEventCallMapping[0.4f] = 
		[this](Engine::SkeletonMesh * const SkMesh) {
		GetComponent<Engine::Transform>()->ClearVelocity();
	};

	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::Dash;
	FSMControlInfo.MyTransform->AddVelocity(DashDirection * StateableSpeed.Dash);
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
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();
	
	if (  ( CurLocation.y  - LandCheckHighRange  ) < Physic.CurrentGroundY)
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
	WeaponAttachImplementation(this, "Weapon_Hand_R",
		{ 1,1,1 },
		{ 0,0,0 },
		{ 0,0,0 });
};



void Player::DashComboState(const FSMControlInformation& FSMControlInfo)&
{
	const bool bAttackMotionEnd =
		(FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime() > 0.5f);

	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return; 
	}

	if (CheckTheJumpableState(FSMControlInfo))
	{
		JumpStartTransition(FSMControlInfo);
		return;
	}
}

void Player::StandUpRollingState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		return;
	}
	const float CurNormalzieAnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (CurNormalzieAnimTime > 0.3f)
	{
		if (auto bMoveable = CheckTheMoveableState(FSMControlInfo);
			bMoveable)
		{
			RunTransition(FSMControlInfo, std::nullopt);
			return;
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

	CurrentMoveDirection = ControlMoveDirection;
	CurrentMoveDirection.y = 0.0f;
	DashDirection = CurrentMoveDirection = FMath::Normalize(CurrentMoveDirection);
	Vector3 NewRotation = FSMControlInfo.MyTransform->GetRotation();
	const float Yaw = std::atan2f(CurrentMoveDirection.x, CurrentMoveDirection.z);
	NewRotation.y = Yaw - FMath::PI / 2.f;
	FSMControlInfo.MyTransform->SetRotation(NewRotation);

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandUpRolling";
	_AnimNotify.AnimTimeEventCallMapping[0.3f] =
		[this](Engine::SkeletonMesh* const SkMesh) {
		GetComponent<Engine::Transform>()->ClearVelocity();
	};

	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandUpRolling;
	FSMControlInfo.MyTransform->AddVelocity(DashDirection * StateableSpeed.Rolling);
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
}
void Player::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);


	auto _Transform = GetComponent<Engine::Transform>();
	const Vector3 Location = _Transform->GetLocation();

	if (CurrentCell == nullptr)
	{
		auto & NaviMesh = RefNaviMesh();
		CurrentCell=NaviMesh.GetCellFromXZLocation({Location.x , Location.z});
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
				_Transform->RefPhysic().CurrentGroundY = bCellResult->Y;
				ImGui::Text("State : Move ,  Y : %.3f , Address : %d", bCellResult->Y, CurrentCell);
			}
			else if (_CompareType == Engine::Cell::CompareType::Stop)
			{
				CurrentCell = bCellResult->Target;
				_Transform->RefPhysic().CurrentGroundY = -1000.f;
				ImGui::Text("State : Stop , Y : %.3f , Address : %d", bCellResult->Y, CurrentCell);
			}
		}
	}

	if (Location.y < 0.0f)
	{
		auto& Control = RefControl();
		FSMControlInformation FSMControlInfo{ _Transform  ,Control, GetComponent<Engine::SkeletonMesh>(), DeltaTime };
		JumpDownTransition(FSMControlInfo);
	}

	

	if (Location.y < -900.f)
	{
		_Transform->SetLocation(_Transform->RefPhysic().GetLastLandLocation());
		
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

void Player::LeafAttackInformation::Reset(
	const Vector3& StartLocation, 
	const Vector3& DestLocation, 
	const float Heighest,
	const float HeighestTime, 
	const float t)&
{
	this->StartLocation = StartLocation;
	this->DestLocation= DestLocation;
	this->Heighest = Heighest;
	this->HeighestTime = HeighestTime;
	this->t = t;

	PreCalculate();
}

void Player::LeafAttackInformation::PreCalculate()&
{
	DestHeight = DestLocation.y - StartLocation.y;
	ReachHeightTime = Heighest - StartLocation.y;
	Gravity= 2.f * ReachHeightTime / (HeighestTime * HeighestTime);

	Velocity.y = std::sqrtf(2.f * Gravity * ReachHeightTime);

	float b = -2.f* Velocity.y;
	float c = 2.f * DestHeight;

	ReachDestLocationTime = (-b + std::sinf(b * b - 4.f * Gravity * c)) / (2.f * Gravity);

	Velocity.x = -(StartLocation.x - DestLocation.x) / ReachDestLocationTime;
	Velocity.z = -(StartLocation.z - DestLocation.z) / ReachDestLocationTime;

	//Velocity.x *= 1.f;
	//Velocity.z *= 1.f;
	// Velocity.y *= 2.f;
}

std::optional<Vector3 > Player::LeafAttackInformation::Move(const float DeltaTime)
{
	t += DeltaTime;
	// 도착 예정시간을 넘어섰음.
	if (t >  (ReachDestLocationTime))return {};

	Vector3 MoveLocation;

	const float YAxisTime = t * 2.f;

	MoveLocation.x = StartLocation.x + Velocity.x * t;
	MoveLocation.z = StartLocation.z + Velocity.z * t;
	MoveLocation.y = StartLocation.y + Velocity.y * YAxisTime - 0.5f * Gravity* YAxisTime * YAxisTime;

	return  { MoveLocation };
}
