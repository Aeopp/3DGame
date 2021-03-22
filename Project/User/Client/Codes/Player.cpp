#include "..\\stdafx.h"
#include "Player.h"
#include "Transform.h"
#include "Timer.h"
#include "DynamicMesh.h"
#include "Renderer.h"
#include "Monster.h"
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
#include "NPC.h"
#include "StaticMesh.h"
#include "EffectSystem.h"
#include "FontManager.h"
#include "ObjectEdit.h"
#include "Landscape.h"
#include "Sound.h"

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
	_Transform->bLastLandUpdate = true;

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
			Engine::CollisionTag::NPC ,
			Engine::CollisionTag::Enemy ,
			Engine::CollisionTag::EnemyAttack
		});

	_Collision->RefPushCollisionables().insert(
		{
	       Engine::CollisionTag::Enemy
		});

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

	PlayerTargetInfo.CurrentDistancebetweenTarget = 780.f;
	PlayerTargetInfo.DistancebetweenTarget = 644.f;
	PlayerTargetInfo.TargetLocationOffset = PlayerCameraTargetLocationOffset;
	PlayerTargetInfo.TargetObject = this;
	PlayerTargetInfo.ViewDirection = { -0.224299625f,-0.518758416f,0.824972391f};
	PlayerTargetInfo.CurrentViewDirection = {-0.224303931f,-0.518762290f,0.824968755f};
	PlayerTargetInfo.CurrentTargetLocation = { 102.264305f,28.0000954f,-100.273598f};
	PlayerTargetInfo.RotateResponsiveness = 0.01f;
	PlayerTargetInfo.ZoomInOutScale = 0.1f;
	PlayerTargetInfo.MaxDistancebetweenTarget = 150.f;

	if (Engine::Global::bDebugMode == false)
	{
		bControl = true;

		CurrentTPCamera = Manager.NewObject<Engine::NormalLayer,
			Engine::ThirdPersonCamera>(L"Static", L"ThirdPersonCamera",
				FMath::PI / 4.f, 0.1f, 1000.f, Aspect).get();

		Manager.FindObject<Engine::NormalLayer, Engine::ThirdPersonCamera>(L"ThirdPersonCamera")->SetUpTarget(PlayerTargetInfo);
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
		InitPhysic.CurrentGroundY = bCellResult->ProjectLocation.y;
	};

	_Transform->EnablePhysic(InitPhysic);

	{
		_BasicCombo01 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		_BasicCombo01->_CurAnimEffectInfo.bRender = false;
	}

	{
		_BasicCombo02 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo02", Engine::EffectSystem::EffectType::AnimEffect);
		_BasicCombo02->_CurAnimEffectInfo.bRender = false;
	}

	{
		_BasicCombo03_1 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo03", Engine::EffectSystem::EffectType::AnimEffect);
		_BasicCombo03_1->_CurAnimEffectInfo.bRender = false;

		_BasicCombo03_2 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo03", Engine::EffectSystem::EffectType::AnimEffect);
		_BasicCombo03_2->_CurAnimEffectInfo.bRender = false;
	}

	{
		_Ex01_1 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		_Ex01_1->_CurAnimEffectInfo.bRender = false;

		_Ex01_2 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		_Ex01_2->_CurAnimEffectInfo.bRender = false;
	}
	
	{
		_Ex02_Start01 = RefRenderer().RefEffectSystem().MakeEffect(L"Combo_Ex02_Start01", Engine::EffectSystem::EffectType::AnimEffect);
		_Ex02_Start01->_CurAnimEffectInfo.bRender = false;
	}

	{
		_Ex02_Loop = RefRenderer().RefEffectSystem().MakeEffect(L"Combo_Ex02_Loop", Engine::EffectSystem::EffectType::AnimEffect);
		_Ex02_Loop->_CurAnimEffectInfo.bRender = false;
	}

	{
		_Ex02_End = RefRenderer().RefEffectSystem().MakeEffect(L"Combo_Ex02_End", Engine::EffectSystem::EffectType::AnimEffect);
		_Ex02_End->_CurAnimEffectInfo.bRender = false;
	}

	{
		AirCombo03 = RefRenderer().RefEffectSystem().MakeEffect(L"Combo_Air03", Engine::EffectSystem::EffectType::AnimEffect);
		AirCombo03->_CurAnimEffectInfo.bRender = false;
		  
		AirCombo01 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		AirCombo01->_CurAnimEffectInfo.bRender = false;

		AirCombo02 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		AirCombo02->_CurAnimEffectInfo.bRender = false;

		AirCombo04 = RefRenderer().RefEffectSystem().MakeEffect(L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);
		AirCombo04->_CurAnimEffectInfo.bRender = false;
	}

	{
		LeapAttack01 = RefRenderer().RefEffectSystem().MakeEffect(L"LeapAttack01", Engine::EffectSystem::EffectType::AnimEffect);
		LeapAttack01->_CurAnimEffectInfo.bRender = false;
		LeapAttack01->_CurAnimEffectInfo.bAnimation = false;
	}

	CreatePlayerSkillUI();
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

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_BasicCombo_01_Renewal.fbx", L"BasicCombo01", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_BasicCombo_02_Renewal.fbx", L"BasicCombo02", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_BasicCombo_03_Renewal.fbx", L"BasicCombo03", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_Combo_Ex01_Renewal.fbx", L"Combo_Ex01", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_Combo_Ex02_Start01_Renewal.fbx", L"Combo_Ex02_Start01", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_Combo_Ex02_Loop_Renewal.fbx", L"Combo_Ex02_Loop", Engine::EffectSystem::EffectType::AnimEffect);
	
	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_BasicComboEX2_02_End.fbx", L"Combo_Ex02_End", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_TS_Combo_Air03_Renewal.fbx", L"Combo_Air03", Engine::EffectSystem::EffectType::AnimEffect);

	RefRenderer().RefEffectSystem().LoadEffect
	(Device, App::ResourcePath / L"Effect" / L"SK_LeapAttack_Ready_01.fbx", L"LeapAttack01", Engine::EffectSystem::EffectType::AnimEffect);
}

void Player::Event()&
{
	Super::Event();
	auto _Transform = GetComponent<Engine::Transform>();
	Edit();

	if (bWeaponAcquisition == false)
	{
		auto& _Manager = RefManager();
		auto  Weapons = _Manager.FindObjects<Engine::NormalLayer, PlayerWeapon>();
		for (auto& EquipTargetWeapon : Weapons)
		{
			if (EquipTargetWeapon->GetName().find(L"PlayerWeapon") != std::wstring::npos)
			{
				const Vector3 WeaponLocation =
					EquipTargetWeapon->GetComponent<Engine::Transform>()->GetLocation();

				if (FMath::Length(WeaponLocation - _Transform->GetLocation()) < 20.f)
				{
					WeaponAcquisition();
					const float WeaponAcquisitionSpeed = (1.0f / 3.f); 
					auto& _Manager = RefManager();
					auto Weapon = _Manager.FindObject<Engine::NormalLayer, PlayerWeapon>(GetName() + L"_Weapon");
					Weapon->DissolveStart(-WeaponAcquisitionSpeed, 1.f);

					auto _NPC =
						dynamic_cast<NPC* const>(RefManager().FindObjects<Engine::NormalLayer, NPC>().front().get());
					_NPC->NextEvent();

					EquipTargetWeapon->DissolveStart(WeaponAcquisitionSpeed, 0.0f);
					RefTimer().TimerRegist(3.0f, 0.0f, 6.1f, [EquipTargetWeapon]() {
						EquipTargetWeapon->Kill();
						return true;  });
				}
			}
		}
	}

	if (FMath::Length( _Transform->GetLocation() - Vector3{154.f,16.f,1.f}) < 10.f)
	{
		if (auto _NPC =
			dynamic_cast<NPC* const>(RefManager().FindObjects<Engine::NormalLayer, NPC>().front().get());
			_NPC)
		{
			if (!_NPC->bTutorialWaveStart &&
				_NPC->CurrentEvent == NPC::InteractionEvent::SecondEncounter &&
				_NPC->CurrentEventIndex == _NPC->EventMents[NPC::InteractionEvent::SecondEncounter].size() - 1u)
			{
				_NPC->bTutorialWaveStart = true;

				ObjectEdit::CaptureObjectLoad(
					App::ResourcePath / "SceneObjectCapture" / "BelatosWaveStart.json");
			}
		}
	}
	

	auto& _Control = RefControl();

	if (_Control.IsDown(DIK_DELETE))
	{
		_LeafAttackInfo.Reset(_Transform->GetLocation(), _Transform->GetLocation() + FMath::Random(Vector3{ -100,99.f,-100 }, Vector3{ 100,100,100 }) ,100.f,1.f);
	}

	auto _SkeletonMeshComponent = GetComponent<Engine::SkeletonMesh>();
	
	if (_Control.IsDown(DIK_O))
	{
		bControl = !bControl;
	}

	if (_Control.IsDown(DIK_F5) )
	{
		ObjectEdit::CaptureObjectLoad(
			App::ResourcePath / "SceneObjectCapture" / "BelatosWaveStart.json");
	}
}

void Player::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	CurrentContinuousAttackCorrectionTime -= DeltaTime;
	FSM(DeltaTime);
	auto _CenterLineQuad = CenterLineQuad.lock();
	_CenterLineQuad->AlphaFactor -= DeltaTime * CenterLineQuadAlphaFactorAcceleration;
	_CenterLineQuad->bRender = true;

	auto _ScreenBloodQuad = ScreenBloodQuad.lock();
	_ScreenBloodQuad->AlphaFactor -= DeltaTime * ScreenBloodQuadAlphaFactorAcceleration;
	_ScreenBloodQuad->bRender = true;

	auto _Transform = GetComponent<Engine::Transform>();

	if (auto bMoveLocation = _LeafAttackInfo.Move(DeltaTime))
	{
		_Transform->SetLocation(*bMoveLocation);
	}

	const Vector3 Location = _Transform->GetLocation();

	auto& _RefPhysic = _Transform->RefPhysic();
	auto& Control    = RefControl();

	const float CoolTimeHeight = 1.f - (std::fabsf(CurrentStandUpRollingCoolTime) / StandUpRollingCoolTime);
	AvoidSlot.lock()->CoolTimeHeight = AvoidIcon.lock()->CoolTimeHeight = CoolTimeHeight;

	DoubleSlashSlot.lock()->CoolTimeHeight = CoolTimeHeight;
	DoubleSlashIcon.lock()->CoolTimeHeight = CoolTimeHeight;

	LeapAttackSlot.lock()->CoolTimeHeight = CoolTimeHeight;
	LeapAttackIcon.lock()->CoolTimeHeight = CoolTimeHeight;

	OutRangeSlot.lock()->CoolTimeHeight   = CoolTimeHeight;
	OutRangeIcon.lock()->CoolTimeHeight   = CoolTimeHeight;

	RockBreakSlot.lock()->CoolTimeHeight  = CoolTimeHeight; 
	RockBreakIcon.lock()->CoolTimeHeight  = CoolTimeHeight;  

	RockShotSlot.lock()->CoolTimeHeight   = CoolTimeHeight;
	RockShotIcon.lock()->CoolTimeHeight   = CoolTimeHeight;

	if (bWeaponAcquisition)
	{
		auto _KarmaIcon = KarmaIcon.lock();

		
		{
			static constexpr float KarmaEventTimeFactor = 0.33f;
			_KarmaIcon->CoolTimeHeight += DeltaTime * KarmaEventTimeFactor;
			auto _PlayerKarmaInfoGUI = PlayerKarmaInfoGUI.lock();

			static bool bPlayerKarmaInfoGUIAlphaUp = true;
			if (bPlayerKarmaInfoGUIAlphaUp)
			{
				_PlayerKarmaInfoGUI->AlphaFactor += (DeltaTime * KarmaEventTimeFactor * 1.f); ;
				if (_PlayerKarmaInfoGUI->AlphaFactor > 1.f)
				{
					bPlayerKarmaInfoGUIAlphaUp = false;
				}
			}
			else
			{
				_PlayerKarmaInfoGUI->AlphaFactor -= (DeltaTime * KarmaEventTimeFactor * 1.f);
			}

			Matrix Billboard = FMath::Inverse(RefRenderer().GetCurrentRenderInformation().View);
			Billboard._41 = 0.0f;
			Billboard._42 = 0.0f;
			Billboard._43 = 0.0f;
			const Vector3 UILocation = _Transform->GetLocation() +
				Vector3{ 0, 9,0 };

			_PlayerKarmaInfoGUI->WorldUI =
				FMath::Scale({ 19,19,1 }) * Billboard *
				FMath::Translation(UILocation);

			_PlayerKarmaInfoGUI->WorldUI->_42 += 22.f;
		}
	}


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

		if (ImGui::Button("WeaponEquip"))
		{
			WeaponAcquisition();
		}

		ImGui::SliderFloat("LeafAttackAxisDelta", &LeafAttackInformation::LeafAttackAxisDelta, 0.f, 1.f);
	/*	ImGui::SliderFloat("LeafAttackHightest", &StateableSpeed.LeafAttackHightest, 0.f, 200.f);
		ImGui::SliderFloat("LeafAttackHightestTime", &StateableSpeed.LeafAttackHightestTime, 0.f, 1.f);*/

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

		ImGui::SliderFloat3("NPCInteractionLocationOffset", NPCInteractionLocationOffset, -50.f, 50.f);

		if (ImGui::TreeNode("Teleport"))
		{
			if (ImGui::SmallButton("Front Weapon"))
			{
				static const Vector3 WeaponFront = { -2.4,87.f,-420.520f };
				_Transform->SetLocation(WeaponFront);

				auto& NaviMesh = RefNaviMesh();
				CurrentCell = NaviMesh.GetCellFromXZLocation({ 
					WeaponFront.x, WeaponFront.z});
			}
			if (ImGui::SmallButton("Front NPC"))
			{
				static const Vector3 NPCFront = { 367.f,-2.637f,10.318f};
				_Transform->SetLocation(NPCFront);

				auto& NaviMesh = RefNaviMesh();
				CurrentCell = NaviMesh.GetCellFromXZLocation({
					NPCFront.x, NPCFront.z });
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("AttackForceInformation"))
		{
			static constexpr float Min = 0.0f;
			static constexpr float Max = 1000.f;

			ImGui::SliderFloat("BasicCombo", &_AttackForce.BasicCombo, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("BasicComboEx03_1", &_AttackForce.BasicComboEx03_1, Min, Max);
			ImGui::SliderFloat("BasicComboEx03_1_Jump", &_AttackForce.BasicComboEx03_1_Jump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("BasicComboEx03_2", &_AttackForce.BasicComboEx03_2, Min, Max);
			ImGui::SliderFloat("BasicComboEx03_2_Jump", &_AttackForce.BasicComboEx03_2_Jump, Min, Max);
			ImGui::Separator();
			
			ImGui::SliderFloat("Dash", &_AttackForce.Dash, Min, Max);
			ImGui::SliderFloat("DashComboJump", &_AttackForce.DashComboJump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Ex01firstCombo", &_AttackForce.Ex01firstCombo, Min, Max);
			ImGui::SliderFloat("Ex01firstComboJump", &_AttackForce.Ex01firstComboJump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Ex01SecondCombo", &_AttackForce.Ex01SecondCombo, Min, Max);
			ImGui::SliderFloat("Ex01SecondComboJump", &_AttackForce.Ex01SecondComboJump, Min, Max);
			ImGui::Separator();

			
			ImGui::SliderFloat("Ex02End", &_AttackForce.Ex02End, Min, Max);
			ImGui::SliderFloat("Ex02EndJump", &_AttackForce.Ex02EndJump, Min, Max);
			ImGui::Separator();
			
			ImGui::SliderFloat("Ex02Loop", &_AttackForce.Ex02Loop, Min, Max);
			ImGui::SliderFloat("Ex02LoopJump", &_AttackForce.Ex02LoopJump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Ex02Start", &_AttackForce.Ex02Start, Min, Max);
			ImGui::SliderFloat("Ex02StartJump", &_AttackForce.Ex02StartJump, Min, Max);
			ImGui::Separator();


			ImGui::SliderFloat("Air01", &_AttackForce.Air01, Min, Max);
			ImGui::SliderFloat("Air01Jump", &_AttackForce.Air01Jump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Air02", &_AttackForce.Air02, Min, Max);
			ImGui::SliderFloat("Air02Jump", &_AttackForce.Air02Jump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Air03", &_AttackForce.Air03, Min, Max);
			ImGui::SliderFloat("Air03Jump", &_AttackForce.Air03Jump, Min, Max);
			ImGui::Separator();

			ImGui::SliderFloat("Air04", &_AttackForce.Air04, Min, Max);
			ImGui::SliderFloat("Air04Jump", &_AttackForce.Air04Jump, Min, Max);
			ImGui::Separator();



			ImGui::TreePop();
		}
	}
};

void Player::FSM(const float DeltaTime)&
{
	auto& Control =      RefControl();
	auto _Transform =    GetComponent<Engine::Transform>();
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
			if (CurrentStandUpRollingCoolTime <= 0.0f)
			{
				CurrentStandUpRollingCoolTime = StandUpRollingCoolTime;
				StandUpRollingTransition(FSMControlInfo, *bMoveInfo);
			}
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

	//if (FSMControlInfo._Controller.IsDown(DIK_J) && bControl)
	//{
	//	StandBigFrontTransition(FSMControlInfo);
	//}
	//else if(FSMControlInfo._Controller.IsDown(DIK_L) && bControl)
	//{
	//	StandBigBackTransition(FSMControlInfo);
	//}
	//else if (FSMControlInfo._Controller.IsDown(DIK_L) && bControl)
	//{
	//	StandBigLeftTransition(FSMControlInfo);
	//}
	//else if (FSMControlInfo._Controller.IsDown(DIK_O) && bControl)
	//{
	//	StandBigRightTransition(FSMControlInfo);
	//}

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

	if (CheckTheAttackableState(FSMControlInfo))
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

	if (CheckTheAttackableState(FSMControlInfo))
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

	if (CheckTheAttackableState(FSMControlInfo))
	{
		AirCombo01Transition(FSMControlInfo);
		return;
	}

	auto _Transform = FSMControlInfo.MyTransform;
	auto& CurLocation= _Transform->GetLocation();

	const auto& Physic = _Transform->RefPhysic();

	bool bNextJumpMotionChange = false;

	bNextJumpMotionChange |= CheckTheLandingStatable(CurLocation.y, Physic.CurrentGroundY);

	if (bNextJumpMotionChange)
	{
		JumpLandingTransition(FSMControlInfo);
		return;
	}
};

bool Player::CheckTheLandingStatable(const float CurLocationY, const float CurGroundY)&
{
	const float      CorrectionLocationY = (CurLocationY - LandCheckHighRange);
	return (CorrectionLocationY < CurGroundY) && ((CurGroundY - CorrectionLocationY) >= 1.f);
}

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

	if (FMath::IsRange(0.3f, 0.4f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Ex02Start, _AttackForce.Ex02StartJump);	
	}
	else if (FMath::IsRange(0.5f, 0.f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Ex02Start, _AttackForce.Ex02StartJump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}


	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.ComboEx02);
	}

	if (!FSMControlInfo._Controller.IsPressing(DIK_MIDCLICK))
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart(); 
		CombatWaitTransition(FSMControlInfo);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		ComboEx02LoopTransition(FSMControlInfo);
		return;
	}
}; 

void Player::ComboEx02StartTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "ComboEx02Start";

	;

	_AnimNotify.AnimTimeEventCallMapping[0.27f] = 
		[this ,FSMControlInfo](Engine::SkeletonMesh*)
	{
		SwordCameraShake();
		SwordEffectPlay(_Ex02_Start01, FSMControlInfo, Vector3{ FMath::ToRadian(-16.5f),0.f,FMath::ToRadian(22.5f) },
			{ 0,0,0 }, 0.9f, false, true, 1.f,{ 0.f,1.f });
	};

	_AnimNotify.AnimTimeEventCallMapping[0.58f] =
		[this, FSMControlInfo](Engine::SkeletonMesh*)
	{
		SwordCameraShake();
		SwordEffectPlay(_Ex02_Loop, FSMControlInfo, Vector3{ 0,0,0 }, Vector3{ 0,5.f,0 },1.f     , false, true, 1.f,{ 0.f,1.f } );
	};

	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::ComboEx02Start;
	SwordCameraShake();
}; 

void Player::ComboEx02LoopState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	GetWeapon()->StartAttack(this, _AttackForce.Ex02Loop, _AttackForce.Ex02LoopJump);

	SwordCameraShake(0.25f,FSMControlInfo.DeltaTime+0.0001f);

	_Ex02_Loop->Location = FSMControlInfo.MyTransform->GetLocation() + Vector3{ 0.f,10.f,0.f };

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.ComboEx02);
	};

	if (CurAnimNotify.bAnimationEnd ||
		!FSMControlInfo._Controller.IsPressing(DIK_MIDCLICK) )
	{
		_Ex02_Loop->_CurAnimEffectInfo.bRender = false;
		GetWeapon()->EndAttack(this);
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

	
	SwordEffectPlay(_Ex02_Loop, FSMControlInfo, Vector3{ 0.f,0.f,0.f }, Vector3{ 0,0,0 },35.f/30.f,true,false);
}; 

void Player::ComboEx02EndState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.08, 0.145f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Ex02End, _AttackForce.Ex02EndJump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (CurAnimNotify.bAnimationEnd )
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
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

	SwordEffectPlay(_Ex02_End, FSMControlInfo, Vector3{ FMath::ToRadian(-22.5f),0.f,FMath::ToRadian(0.f) }, { 0,0,0 },0.9f ,
		false,true ,1.f,
		{ 0.f,1.f });
	SwordCameraShake();
};


void Player::ComboEx01State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.12, 0.17, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this,_AttackForce.Ex01firstCombo,_AttackForce.Ex01firstComboJump);
		SwordCameraShake(0.88f, FSMControlInfo.DeltaTime);
		SwordEffectPlay(_Ex01_1, FSMControlInfo, Vector3{ FMath::ToRadian(90.f),0.f,0.f }, { 0.f,0.f,0.f }, 2.f);
	}
	else if (FMath::IsRange(0.20, 0.25, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Ex01SecondCombo, _AttackForce.Ex01SecondComboJump);
		SwordCameraShake(2.f, FSMControlInfo.DeltaTime);
		SwordEffectPlay(_Ex01_2, FSMControlInfo, Vector3{ FMath::ToRadian(63.f),0.f,FMath::ToRadian(12.0f) }, { 0.f,0.f,0.f }, 1.5f,
			false, true, 1.f,
			{ 1.f,1.f });
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (CheckTheJumpableState(FSMControlInfo))
		{
			GetWeapon()->EndAttack(this);
			WeaponPutDissolveStart();
			JumpStartTransition(FSMControlInfo);
			return;
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
		CombatWaitTransition(FSMControlInfo);
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

PlayerWeapon* const  Player:: GetWeapon()const & 
{
	return RefManager().FindObject<Engine::NormalLayer, PlayerWeapon>(Name + L"_Weapon");
}

void Player::BasicCombo01State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify  = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.08f, 0.13f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.BasicCombo,0.0f);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}


	if (AnimTimeNormal < 0.7f)
	{
		if (CheckTheAttackableState(FSMControlInfo))
		{
			CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;

			GetWeapon()->EndAttack(this);
			BasicCombo02Transition(FSMControlInfo);
			return;
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
		CombatWaitTransition(FSMControlInfo);
		return;
	}
}


void Player::AirCombo01State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();
	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | CheckTheLandingStatable(Location.y, Physic.CurrentGroundY));
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.16f, 0.235f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Air01,_AttackForce.Air01Jump );
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (bLandingChange)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();

		JumpLandingTransition(FSMControlInfo);
		return;
	}

	if (CheckTheAttackableState(FSMControlInfo))
	{
		CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;
		GetWeapon()->EndAttack(this);
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
	
	SwordCameraShake(7.f, 0.25f);
	SwordEffectPlay(AirCombo01, FSMControlInfo, Vector3{ FMath::ToRadian(-35.f),0.f,FMath::ToRadian(11.f) } ,
		{ 0,12.f,0 },
		1.f, false, true, 1.f,
		{ 1.f,1.f });
};

void Player::AirCombo02State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();


	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();

	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.18f, 0.35f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Air02,_AttackForce.Air02Jump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	
	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | CheckTheLandingStatable(Location.y, Physic.CurrentGroundY));

	if (bLandingChange)
	{
		WeaponPutDissolveStart();
		GetWeapon()->EndAttack(this);
		JumpLandingTransition(FSMControlInfo);
		return;
	}

	if (CheckTheAttackableState(FSMControlInfo))
	{
		CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;
		GetWeapon()->EndAttack(this);
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

	SwordCameraShake(7.f, 0.25f);
	SwordEffectPlay(AirCombo02, FSMControlInfo, Vector3{ FMath::ToRadian(35.f),0.f,FMath::ToRadian(11.f) },
		{ 0,12.f,0 },
		1.f, false, true, 1.f,
		{ 1.f,1.f });

};
void Player::AirCombo03State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto& Physic = FSMControlInfo.MyTransform->RefPhysic();
	
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.06f, 0.23f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Air03, _AttackForce.Air03Jump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	const bool bLandingChange = (CurAnimNotify.bAnimationEnd | CheckTheLandingStatable(Location.y, Physic.CurrentGroundY));
	if (bLandingChange)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
		JumpLandingTransition(FSMControlInfo);
		return;
	}



	if (CheckTheAttackableState(FSMControlInfo))
	{
		CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;
		GetWeapon()->EndAttack(this);
		AirCombo04Transition(FSMControlInfo);
		return;
	};


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

	SwordCameraShake(7.f, 0.25f);
	SwordEffectPlay(AirCombo03, FSMControlInfo, Vector3{ FMath::ToRadian(90.f),0.f,FMath::ToRadian(13.5f) },
		{ 0,12.f,0 },
		1.f, false, true, 1.f,
		{ 1.f,1.f });

};
void Player::AirCombo04State(const FSMControlInformation& FSMControlInfo)& 
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	
	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const auto & Physic  = FSMControlInfo.MyTransform->RefPhysic();

	bool bLandingChange = (CurAnimNotify.bAnimationEnd | CheckTheLandingStatable(Location.y, Physic.CurrentGroundY));
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	AirCombo04->Location = FSMControlInfo.MyTransform->GetLocation() + Vector3{ 0.f,12.f,0.f };
	if (FMath::IsRange(0.3f, 1.0f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.Air04, _AttackForce.Air04Jump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (bLandingChange)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
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

	SwordCameraShake(7.f, 0.35f);
	SwordEffectPlay(AirCombo04, FSMControlInfo, Vector3{ FMath::ToRadian(-90.f),FMath::ToRadian ( 7.00f),FMath::ToRadian(7.00f) },
		{ 0,0,0 },
		1.f/2.f, false, true, 1.f,
		{ 1.f,1.f });
};
void Player::AirCombo04LandingState(const FSMControlInformation& FSMControlInfo)& 
{
	const float AnimTime = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	if (AnimTime > 0.97f )
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
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
	_AnimNotify.bLoop = true;


	
	SwordEffectPlay(LeapAttack01, FSMControlInfo, Vector3{ FMath::ToRadian(-22.5f),0.f,FMath::ToRadian(22.5f) });
	
	SwordEffectPlay(_BasicCombo01, FSMControlInfo, Vector3{ FMath::ToRadian(-22.5f),0.f,FMath::ToRadian(22.5f) });
	SwordCameraShake();

}

void Player::BasicCombo02State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify  = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.14f, 0.18f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.BasicCombo,0.0f);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (AnimTimeNormal < 0.7f)
	{
		if (CheckTheAttackableState(FSMControlInfo))
		{
			CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;
			GetWeapon()->EndAttack(this);
			BasicCombo03Transition(FSMControlInfo);
			return;
		};

		if (FSMControlInfo._Controller.IsDown(DIK_RIGHTCLICK) && bControl)
		{
			GetWeapon()->EndAttack(this);
			ComboEx01Transition(FSMControlInfo);
			return;
		}

		if (FSMControlInfo._Controller.IsPressing(DIK_MIDCLICK))
		{
			GetWeapon()->EndAttack(this);
			ComboEx02StartTransition(FSMControlInfo);
			return;
		}
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
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

	SwordEffectPlay(_BasicCombo02, FSMControlInfo, Vector3{ 0.f,0.f,FMath::ToRadian(12.0f) }, Vector3{0,0.f,0.f});
	SwordCameraShake();
}

void Player::BasicCombo03State(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.08f, 0.15f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this ,_AttackForce.BasicComboEx03_1, _AttackForce.BasicComboEx03_1_Jump);
		SwordCameraShake(0.88f, FSMControlInfo.DeltaTime);
		SwordEffectPlay(_BasicCombo03_1, FSMControlInfo, Vector3{ 0.f,0.f,0.f } , {0.f,5.f,0.f}, 1.0f );
	}
	else if (FMath::IsRange(0.17f, 0.24f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this, _AttackForce.BasicComboEx03_2, _AttackForce.BasicComboEx03_2_Jump);
		SwordCameraShake(2.f, FSMControlInfo.DeltaTime);
		SwordEffectPlay(_BasicCombo03_2, FSMControlInfo, Vector3{ FMath::ToRadian(41.5f),0.f,FMath::ToRadian(12.0f) }, {}, 1.5f );
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}

	if (auto bMoveInfo = CheckTheMoveableState(FSMControlInfo);
		bMoveInfo)
	{
		MoveFromController(FSMControlInfo, *bMoveInfo, StateableSpeed.Attack);
	}

	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
		CombatWaitTransition(FSMControlInfo);
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
	
	/*SwordEffectPlay(_BasicCombo03, FSMControlInfo, Vector3{ 0.f,0.f,FMath::ToRadian(0.0f) });
	SwordCameraShake();*/
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

	auto _CenterLineQuad = CenterLineQuad.lock();
	_CenterLineQuad->AlphaFactor = 1.f;
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
	bInvincibility = true;


	auto _ScreenBloodQuad =ScreenBloodQuad.lock();
	_ScreenBloodQuad->AlphaFactor = 1.f;
}

void Player::StandBigBackState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		bInvincibility = false;
		CombatWaitTransition(FSMControlInfo);
	}
};

void Player::StandBigFrontTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponPut();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "StandBigFront";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::StandBigFront;
	bInvincibility = true;

	auto _ScreenBloodQuad = ScreenBloodQuad.lock();
	_ScreenBloodQuad->AlphaFactor = 1.f;
};

void Player::StandBigFrontState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		bInvincibility = false;
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
	bInvincibility = true;

	auto _ScreenBloodQuad = ScreenBloodQuad.lock();
	_ScreenBloodQuad->AlphaFactor = 1.f;
}

void Player::StandBigLeftState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		bInvincibility = false;
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
	bInvincibility = true;

	auto _ScreenBloodQuad = ScreenBloodQuad.lock();
	_ScreenBloodQuad->AlphaFactor = 1.f;
};

void Player::StandBigRightState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	if (CurAnimNotify.bAnimationEnd)
	{
		CombatWaitTransition(FSMControlInfo);
		bInvincibility = false;
	}
};


void Player::LeafAttackReadyState(const FSMControlInformation& FSMControlInfo)&
{
	// 	WeaponPutDissolveStart();

	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	// TODO :: 여기서 움직일 수 있으며 마법진 (?) 을 바닥에 그려주는 인터페이스를 제공하면 좋을듯 !
	if (auto bIsMove = CheckTheMoveableState(FSMControlInfo); bIsMove)
	{
		MoveFromController(FSMControlInfo, *bIsMove, StateableSpeed.LeafReady);
	}
	LeafReadyCameraUpdate(FSMControlInfo);
	
	const float XAxisDelta = FSMControlInfo._Controller.GetMouseMove(Engine::MouseMove::X) * 
		_LeafAttackInfo.LeafAttackAxisDelta;
	const float ZAxisDelta = FSMControlInfo._Controller.GetMouseMove(Engine::MouseMove::Y) * _LeafAttackInfo.LeafAttackAxisDelta;

	const Vector3 Location = FSMControlInfo.MyTransform->GetLocation();
	const Vector3 Right = FSMControlInfo.MyTransform->GetRight();
	const Vector3 Forward = FSMControlInfo.MyTransform->GetForward();

	const Vector2 RightXZ = { Right.x,Right.z };
	const Vector2 ForwardXZ = { Forward.x, Forward.z };

	RefRenderer().RefLandscape().AuraPosition += RightXZ * -ZAxisDelta;
	const Vector2 TargetLocation = RefRenderer().RefLandscape().AuraPosition += ForwardXZ * -XAxisDelta;

	if (FSMControlInfo._Controller.IsUp(DIK_R))
	{
		auto TargetCell = RefNaviMesh().GetCellFromXZLocation(TargetLocation);
		if (TargetCell)
		{
			const Vector3 ProjectionLocation = FMath::ProjectionPointFromFace(
				TargetCell->_Plane._Plane,
				{ TargetLocation.x , 0.0f ,TargetLocation.y });

			_LeafAttackInfo.Reset(
				FSMControlInfo.MyTransform->GetLocation(), ProjectionLocation, 
				80.f,
				1.f);

			const Vector3 LeafAttackDir = FMath::Normalize(ProjectionLocation - FSMControlInfo.MyTransform->GetLocation());
			Vector3 NewRotation = FSMControlInfo.MyTransform->GetRotation();
			const float Yaw = std::atan2f(LeafAttackDir.x, LeafAttackDir.z);
			NewRotation.y = Yaw - FMath::PI / 2.f;
			FSMControlInfo.MyTransform->SetRotation(NewRotation);
			LeafAttackStartTransition(FSMControlInfo);
			return;
		}
	}
};

void Player::LeafAttackReadyTransition(const FSMControlInformation& FSMControlInfo)&
{
	WeaponHand();

	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = true;
	_AnimNotify.Name = "LeafAttackReady";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackReady;

	const Vector3 Location=FSMControlInfo.MyTransform->GetLocation();
	RefRenderer().RefLandscape().AuraPosition =  { Location .x , Location .z};
}

void Player::LeafAttackStartState(const FSMControlInformation& FSMControlInfo)&
{
	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();

	LeafAttackCameraUpdate(FSMControlInfo) ;

	CenterLineQuad.lock()->AlphaFactor = 1.f - (_LeafAttackInfo.t / _LeafAttackInfo.HeighestTime);

	if (CurAnimNotify.bAnimationEnd)
	{
		LeafAttackUpTransition(FSMControlInfo);
		return;
	}
}

void Player::LeafAttackStartTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "LeafAttackStart";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackStart;

	CurrentTPCamera->RefTargetInformation().LocationLerpSpeed = 8.f;
}

void Player::LeafAttackUpState(const FSMControlInformation& FSMControlInfo)&
{
	// 여기서 중력 가속도가  점프 속도를 초과 해서 음수가나올 경우 애니메이션을 전환해야한다 .

	LeafAttackCameraUpdate(FSMControlInfo);
	CenterLineQuad.lock()->AlphaFactor = 1.f - (_LeafAttackInfo.t / _LeafAttackInfo.HeighestTime);
	const auto& _Physic =  FSMControlInfo.MyTransform->RefPhysic();
	if (_Physic.Velocity.y < 0.0f || 
		_LeafAttackInfo.t > _LeafAttackInfo.HeighestTime)
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
	LeafAttackCameraUpdate(FSMControlInfo);
	CenterLineQuad.lock()->AlphaFactor = 1.f - (_LeafAttackInfo.t / _LeafAttackInfo.HeighestTime);
	if (CheckTheLandingStatable(CurLocation.y, Physic.CurrentGroundY) )
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

	LeafAttackCameraUpdate(FSMControlInfo);
	CenterLineQuad.lock()->AlphaFactor = 1.f - (_LeafAttackInfo.t / _LeafAttackInfo.HeighestTime);
	if (CurAnimNotify.bAnimationEnd)
	{
		CenterLineQuad.lock()->AlphaFactor = 0.0f;
		RefRenderer().RefLandscape().AuraPosition = { FLT_MAX,FLT_MAX };
		WeaponPutDissolveStart();
		CombatWaitTransition(FSMControlInfo); 
		return;
	}
}

void Player::LeafAttackLandingTransition(const FSMControlInformation& FSMControlInfo)&
{
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "LeafAttackLanding";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::LeafAttackLanding;

	CurrentTPCamera->RefTargetInformation().LocationLerpSpeed = 8.f;
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
	if (FSMControlInfo._Controller.IsDown(DIK_LEFTCLICK) && bControl 
					&&bWeaponAcquisition && (CurrentContinuousAttackCorrectionTime<0.0f))
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
		PlayerWeapon->SetScale   (OffsetScale);
		PlayerWeapon->SetRotation(OffsetRotation);
		PlayerWeapon->SetLocation(OffsetLocation);
	}
};

void Player::WeaponAcquisition()&
{
	PlayerKarmaInfoGUI.lock()->bRender = true;
	KarmaIcon.lock()->bRender = true;
	bWeaponAcquisition = true;

	StateableSpeed.Jump -= 10.f;
	StateableSpeed.JumpVelocity.y -= 15.f;

	std::shared_ptr<PlayerWeapon> _PlayerWeapon =
		RefManager().NewObject<Engine::NormalLayer,PlayerWeapon>(L"Static",
			Name + L"_Weapon", 
			Vector3{ 1.1f,1.1f,1.1f },
			Vector3{ 0,0,0 }, 
			Vector3{ 0,0,0 });

	RefSound().Play("EFF_OBJ_LogIn_Rune_01_D", 1.f,true);

	WeaponPut();
}

void Player::WeaponPutDissolveStart()&
{
	auto& _Manager = RefManager();
	auto Weapon = _Manager.FindObject<Engine::NormalLayer, PlayerWeapon>(GetName() + L"_Weapon");
	Weapon->DissolveStart(WeaponDissolveTime, 1.f);
}

void Player::WeaponPut()&
{
	WeaponAttachImplementation(this, "Weapon_Pelvis_R", 
		{0.90f,0.90f,0.90f },
		{0,-2.583f,0}, 
		{ -20.263f,42.553f,-88.813f });
};

void Player::WeaponHand()&
{
	WeaponAttachImplementation(this, "Weapon_Hand_R",
		{ 1,1,1 },
		{ 0,0,0 },
		{ 0,0,0 });
};

void Player::LeafReadyCameraUpdate(const FSMControlInformation& FSMControlInfo)&
{
	CurrentTPCamera->RefTargetInformation().ViewDirection =
		FMath::Normalize(FMath::RotationVecNormal(FSMControlInfo.MyTransform->GetRight(),
			 FSMControlInfo.MyTransform->GetForward(), -FMath::ToRadian(22.5f)));
	// CurrentTPCamera->RefTargetInformation().DistancebetweenTarget = 60.f;
};

void Player::LeafAttackCameraUpdate(const FSMControlInformation& FSMControlInfo)&
{
	CurrentTPCamera->RefTargetInformation().ViewDirection =
		FMath::Normalize(FMath::RotationVecNormal(FSMControlInfo.MyTransform->GetRight(),
			FSMControlInfo.MyTransform->GetForward(), -FMath::ToRadian(22.5f)));
	CurrentTPCamera->RefTargetInformation().DistancebetweenTarget = 40.f;
};

void Player::SwordEffectPlay(Engine::AnimEffect* _AnimEffect, const FSMControlInformation& FSMControlInfo,
				const Vector3 RotationOffset, const Vector3 LocationOffset , const float TimeAcceleration,
				const bool bLoop,const bool bLinearAlpha, const float AlphaFactor ,
				const Vector2& GradientUVOffsetFactor,
				const std::wstring& DiffuseMap,
				const std::wstring& PatternMap,
				const std::wstring& AddColorMap,
				const std::wstring& UVDistorMap)&
{
	Engine::AnimEffect::AnimNotify _EftAnimNotify{};
	_EftAnimNotify.bLoop = bLoop;

	_AnimEffect->Scale = FSMControlInfo.MyTransform->GetScale() * 1.1f;
	_AnimEffect->Rotation = FSMControlInfo.MyTransform->GetRotation() + RotationOffset;
	_AnimEffect->Location = FSMControlInfo.MyTransform->GetLocation() + Vector3{ 0,10,0 } + LocationOffset;
	_AnimEffect->PlayAnimation(0u, 30.f * TimeAcceleration, 0.25f, _EftAnimNotify);
	_AnimEffect->_CurAnimEffectInfo.DiffuseMap = RefRenderer().RefEffectSystem().EffectTextures.find(DiffuseMap)->second;
		_AnimEffect->_CurAnimEffectInfo.PatternMap = RefRenderer().RefEffectSystem().EffectTextures.find(PatternMap)->second;
		_AnimEffect->_CurAnimEffectInfo.UVDistorMap = RefRenderer().RefEffectSystem().EffectTextures.find(UVDistorMap)->second;
		_AnimEffect->_CurAnimEffectInfo.AddColorMap = RefRenderer().RefEffectSystem().EffectTextures.find(AddColorMap)->second;

	_AnimEffect->_CurAnimEffectInfo.GradientMap = RefRenderer().RefEffectSystem().EffectTextures.find(L"Gradient")->second;
	
	_AnimEffect->_CurAnimEffectInfo.Time = 0.0f;
	_AnimEffect->_CurAnimEffectInfo.AlphaFactor = 0.0f;
	_AnimEffect->_CurAnimEffectInfo.bRender = true;
	_AnimEffect->_CurAnimEffectInfo.Brightness = 1.00f;
	_AnimEffect->_CurAnimEffectInfo.GradientUVOffsetFactor = GradientUVOffsetFactor;

	

	_AnimEffect->_AnimEffectUpdateCall = [TimeAcceleration , bLoop , bLinearAlpha , _AlphaFactor =AlphaFactor](Engine::AnimEffect::AnimEffectInfo& _EffectInfo, float DeltaTime)
	{
		_EffectInfo.Time += DeltaTime * 1.33f * TimeAcceleration;
		const float Factor = std::fabsf(std::sinf(_EffectInfo.Time));

		if (bLinearAlpha)
		{
			_EffectInfo.AlphaFactor = 1.f - _EffectInfo.Time;
		}
		else
		{
			_EffectInfo.AlphaFactor = 1.f;
		}
		
		_EffectInfo.AlphaFactor *= _AlphaFactor;

		if (_EffectInfo.Time > 1.f)
		{
			if (!bLoop)
			{
				_EffectInfo.bRender = false;
			}
			else
			{
				_EffectInfo.Time = 0.0f;
			}
		}
	};
}
void Player::SwordCameraShake(const float Force,const float Duration)&
{
	CurrentTPCamera->Shake(Force, FMath::Random<Vector3>({ -1,-1,-1 }, { 1,1,1 }), Duration);
};



void Player::DashComboState(const FSMControlInformation& FSMControlInfo)&
{
	const bool bAttackMotionEnd =
		(FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime() > 0.5f);

	const auto& CurAnimNotify = FSMControlInfo.MySkeletonMesh->GetCurrentAnimNotify();
	const float AnimTimeNormal = FSMControlInfo.MySkeletonMesh->GetCurrentNormalizeAnimTime();

	if (FMath::IsRange(0.1f, 0.2f, AnimTimeNormal))
	{
		GetWeapon()->StartAttack(this,_AttackForce.Dash, _AttackForce.DashComboJump);
	}
	else
	{
		GetWeapon()->EndAttack(this);
	}


	if (CurAnimNotify.bAnimationEnd)
	{
		GetWeapon()->EndAttack(this);
		WeaponPutDissolveStart();
		CombatWaitTransition(FSMControlInfo);
		return; 
	}

	if (CheckTheJumpableState(FSMControlInfo))
	{
		GetWeapon()->EndAttack(this);
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

	auto _CenterLineQuad = CenterLineQuad.lock();
	_CenterLineQuad->AlphaFactor = 1.f;
}

void Player::DashComboTransition(const FSMControlInformation& FSMControlInfo)&
{
	CenterLineQuad.lock()->bRender = true;
	WeaponHand();
	Engine::SkeletonMesh::AnimNotify _AnimNotify{};
	_AnimNotify.bLoop = false;
	_AnimNotify.Name = "DashCombo";
	FSMControlInfo.MySkeletonMesh->PlayAnimation(_AnimNotify);
	CurrentState = Player::State::DashCombo;

	SwordCameraShake(8.f,0.4f);
	SwordEffectPlay(_BasicCombo01, FSMControlInfo, Vector3{ FMath::ToRadian(90.f),0.f,0.f }, { 0,0,0 },
		0.9f, false, true, 1.f,
		{ 1.f,1.f } );
}; 

void Player::HitNotify(Object* const Target, const Vector3 PushDir,
	const float CrossAreaScale)&
{
	Super::HitNotify(Target, PushDir, CrossAreaScale);

	auto _Transform = GetComponent<Engine::Transform>();

	auto &Control=RefControl();

	if (auto _NPC = dynamic_cast<NPC* const>(Target);
		_NPC)
	{
		auto& CameraTargetInfo = CurrentTPCamera->RefTargetInformation();
	// 	_NPC->GetComponent<Engine::SkeletonMesh>()->OutlineRedFactor = 0.f;

		if (Control.IsDown(DIK_BACKSPACE))
		{
			_NPC->bInteraction = !_NPC->bInteraction;
			if (_NPC->bInteraction ==false)
			{
				CurrentTPCamera->RefTargetInformation().TargetObject = this;
				CurrentTPCamera->RefTargetInformation().DistancebetweenTarget = 45.f;
				CurrentTPCamera->RefTargetInformation().MaxDistancebetweenTarget = PlayerTargetInfo.MaxDistancebetweenTarget;
				CurrentTPCamera->RefTargetInformation().RotateResponsiveness = PlayerTargetInfo.RotateResponsiveness;
				CurrentTPCamera->RefTargetInformation().TargetLocationOffset = PlayerTargetInfo.TargetLocationOffset;
				CurrentTPCamera->RefTargetInformation().ViewDirection = -_Transform->GetForward();
				CurrentTPCamera->RefTargetInformation().ZoomInOutScale = PlayerTargetInfo.ZoomInOutScale;
			}
		}
		if (_NPC->bInteraction)
		{
			// _NPC->GetComponent<Engine::SkeletonMesh>()->OutlineRedFactor = 0.f;

			CameraTargetInfo.TargetObject = _NPC;
			auto NPCTransform = _NPC->GetComponent<Engine::Transform>();

			Vector3 Distance = (_Transform->GetLocation() + NPCInteractionLocationOffset)
								- (NPCTransform->GetLocation() + _NPC->ViewLocationOffset);

			CameraTargetInfo.TargetLocationOffset = _NPC->ViewLocationOffset;

			const float Distancebetween = FMath::Length(Distance);
			const Vector3 Dir = FMath::Normalize(Distance);

			CameraTargetInfo.DistancebetweenTarget = Distancebetween + 7.f;
			CameraTargetInfo.ViewDirection = -Dir;

			if (Control.IsDown(DIK_L))
			{
				_NPC->NextInteraction();
			}
		}
	}
};

void Player::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::HitBegin(Target, PushDir, CrossAreaScale);
};

void Player::HitEnd(Object* const Target)&
{
	Super::HitEnd(Target);
};

void Player::CreatePlayerSkillUI()&
{
	ShowCursor(false);

	std::filesystem::path UIPath = std::filesystem::current_path();

	  KarmaSlot = RefRenderer().
		MakeUI({ 75.f,75.f }, { -530.f,0.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_KarmaSlot02_N.tga",
			0.1);
	  KarmaSlot.lock()->AddColor = {1.f,1.f,1.f};

	KarmaIcon = RefRenderer().
		MakeUI({ 43.f,43.f }, { -530.f,0.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_Karma_TS.tga",
			0.2);
	KarmaIcon.lock()->Flag = 0u;
	KarmaIcon.lock()->bRender = false;
	KarmaIcon.lock()->CoolTimeHeight = 0.0f;

	DoubleSlashSlot = RefRenderer().
		MakeUI({ 63.f,63.f }, { -550.f,-340.f},
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_Skill_icon_slot.tga",
			0.1);
	DoubleSlashSlot.lock()->AddColor = {1.f,45.f/255.f,45.f/255.f};

	
	DoubleSlashIcon = RefRenderer().
		MakeUI({ 63.f,63.f }, { -550.f,-340.f},
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_RageSkill_TS_DoubleSlash.tga",
			0.2);
	DoubleSlashIcon.lock()->Flag = 0u;

	 LeapAttackSlot = RefRenderer().
		MakeUI({ 38.f,38.f }, { -443.f ,-302.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_HexagonSlot_L_Normal.tga",
			0.1);
	 LeapAttackSlot.lock()->AddColor = Engine::UI::UISkillIconBlue;

	 LeapAttackIcon = RefRenderer().
		MakeUI({ 38.f,38.f }, { -443.f ,-302.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_NormalSkill_TS_LeapAttack.tga",
			0.2);
	 LeapAttackIcon.lock()->Flag = 0u;

	 AvoidSlot = RefRenderer().
		MakeUI({ 38.f,38.f }, { -407.f ,-372.f},
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_HexagonSlot_L_Normal.tga",
			0.1);
	 AvoidSlot.lock()->AddColor = Engine::UI::UISkillIconGreen;

	 AvoidIcon = RefRenderer().
		MakeUI({ 38.f,38.f }, { -407.f ,-372.f},
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_Avoid.tga",
			0.2);
	 AvoidIcon.lock()->Flag = 0u;

	 OutRangeSlot = RefRenderer().
		MakeUI({ 38.f,38.f }, { -354 ,-302},
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_HexagonSlot_L_Normal.tga",
			0.1);
	 OutRangeSlot.lock()->AddColor = Engine::UI::UISkillIconBlue;

	 OutRangeIcon = RefRenderer().
		MakeUI({ 38.f,38.f }, { -354 ,-302 },
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_NormalSkill_TS_OutRage.tga",
			0.2);
	 OutRangeIcon.lock()->Flag = 0u;

	 RockShotSlot = RefRenderer().
		MakeUI({ 38.f,38.f }, { -265 , -302 },
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_HexagonSlot_L_Normal.tga",
			0.1);
	 RockShotSlot.lock()->AddColor = Engine::UI::UISkillIconBlue;

	 RockShotIcon = RefRenderer().
		MakeUI({ 38.f,38.f }, { -265 , -302 },
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_NormalSkill_TS_RockShot.tga",
			0.2);
	 RockShotIcon.lock()->Flag = 0u;

	 RockBreakSlot = RefRenderer().
		MakeUI({ 38.f,38.f }, { -318.f ,-372.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"GUI_HexagonSlot_L_Normal.tga",
			0.1);
	 RockBreakSlot.lock()->AddColor = Engine::UI::UISkillIconRed;

	 RockBreakIcon  = RefRenderer().
		MakeUI({ 38.f,38.f }, { -318.f ,-372.f },
			App::ResourcePath / L"Texture" / L"UI" /
			L"Icon_NormalSkill_TS_RockBreak.tga",
			0.2);
	 RockBreakIcon.lock()->Flag = 0u;

	 PlayerKarmaInfoGUI = RefRenderer().
     MakeUI({ 38.f,38.f }, { -318.f ,-372.f },
			 App::ResourcePath / L"Texture" / L"UI" /
			 L"GUI_karma_player_info_TS_1.tga",
			 0.02);
	 PlayerKarmaInfoGUI.lock()->bRender = false;
	 PlayerKarmaInfoGUI.lock()->WorldUI = FMath::Identity();
	 PlayerKarmaInfoGUI.lock()->AlphaFactor = 0.0f;
	 
	 CenterLineQuad =
		 RefRenderer().MakeUI({ App::ClientSize<float>.first /2.f,
								App::ClientSize<float>.second/2.f}, 
			{0,0},
				App::ResourcePath / L"Texture"/L"UI"/
		 L"Center_Line.tga", 0.9);
	 CenterLineQuad.lock()->bRender = false;
	 CenterLineQuad.lock()->AlphaFactor = 0.0f;

	 ScreenBloodQuad = RefRenderer().MakeUI({ App::ClientSize<float>.first / 2.f ,
											App::ClientSize<float>.second / 2.f },
		 { 0,0 },
		 App::ResourcePath / L"Texture" / L"UI" / L"T_Screen_Blood.tga", 0.9);
	 ScreenBloodQuad.lock()->bRender = false;
	 ScreenBloodQuad.lock()->AlphaFactor = 0.0f;

	 MouseUI = RefRenderer().MakeUI({ 48.f * (App::ClientSize<float>.first/1920.f),
									  48.f * (App::ClientSize<float>.second/1080.f)}, { 0,0 },
		 App::ResourcePath / L"Texture" / L"UI" / L"GUI_MouseCursor_Default.tga", 0.99f);
	 MouseUI.lock()->bRender = false;
	 MouseUI.lock()->AlphaFactor = 1.f;
	 MouseUI.lock()->AddColor = { 0,0,0 };
	 MouseUI.lock()->Flag = 0u;
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

	if (CurrentTPCamera->bCursorMode)
	{
		POINT Pt{};
		GetCursorPos(&Pt);
		ScreenToClient(App::Hwnd, &Pt);
		MouseUI.lock()->Position = {
			static_cast<float>(Pt.x) - App::ClientSize<float>.first / 2.f,
			-static_cast<float>(Pt.y) + (App::ClientSize<float>.second / 2.f) };
		MouseUI.lock()->bRender = true;
	}
	else
	{
		MouseUI.lock()->bRender= false;
	}

	CurrentStandUpRollingCoolTime = (std::max)(CurrentStandUpRollingCoolTime - DeltaTime, 0.0f);

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

				if (Engine::Global::bDebugMode)
				{
					ImGui::Text("State : Move ,  Y : %.3f , Address : %d",
						bCellResult->ProjectLocation.y, CurrentCell);
				}
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
						{ bCellResult->ProjectLocation.x , Location.y ,bCellResult->ProjectLocation.z });
				}
				if (Engine::Global::bDebugMode)
				{
					ImGui::Text("State : Stop , Y : %.3f , Address : %d",
						bCellResult->ProjectLocation.y, CurrentCell);
				};
			}
		}
	}

	if (Location.y < -100.f)
	{
		auto& Control = RefControl();
		FSMControlInformation FSMControlInfo
		{ _Transform  ,Control, GetComponent<Engine::SkeletonMesh>(), DeltaTime };
		JumpDownTransition(FSMControlInfo);
	}

	if (Location.y < -900.f)
	{
		_Transform->SetLocation(_Transform->RefPhysic().GetLastLandLocation());
	}
};

void Player::Hit(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	Super::Hit(Target, PushDir, CrossAreaScale);

	if (auto _Monster = dynamic_cast<Monster* const> (Target);
		_Monster)
	{
		const Vector3 HitDirection = FMath::Normalize(PushDir);
		auto*const _Transform  =GetComponent<Engine::Transform>();
		const float Rad45 = FMath::PI / 4.f;

		const Vector3 Right = _Transform->GetRight();
		const Vector3 Forward = _Transform->GetForward(); 
		
		auto& Control = RefControl();
		auto* const _SkeletonMesh = GetComponent<Engine::SkeletonMesh>();
		const float DeltaTime = RefTimer().GetDelta();  
		HP -= 5.f;
		FSMControlInformation FSMControlInfo{ _Transform  ,Control  ,_SkeletonMesh  , DeltaTime  };
		if (FMath::Dot(-HitDirection , Right) <= Rad45)
		{
			StandBigRightTransition(FSMControlInfo);
		}
		else if (FMath::Dot(-HitDirection, -Right) <= Rad45)
		{
			StandBigLeftTransition(FSMControlInfo); 
		}
		else if (FMath::Dot(-HitDirection, Forward) <= Rad45)
		{
			StandBigFrontTransition(FSMControlInfo);
		}
		else if (FMath::Dot(-HitDirection, -Forward) <= Rad45)
		{
			StandBigBackTransition(FSMControlInfo);
		}
		return;
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









