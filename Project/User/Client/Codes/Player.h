#pragma once
#include <optional>
#include "Object.h"
#include "SkeletonMesh.h"
#include "ThirdPersonCamera.h"

class Player final: public Engine::Object
{
public:
	using Super = Engine::Object;
public:
	struct StateDuringSpeed 
	{
		// 공중에 체공중이라고 판단하는 스피드 (애니메이션을 위함) 
		float InTheAirSpeed = 20.f;

		float Run = 40.f; 
		float ComboEx02 = 8.f;
		float Attack = 8.f; 
		float Jump = 22.2f;
		float Rolling = 40.f;
		float Dash = 100.f;
		float LeafReady = 20.f;

		Vector3 JumpVelocity = { 0.f,110.f,0.f };
		Vector3 AirCombo01Velocity = { 0.f, 70.f,0.f };
		Vector3 AirCombo02Velocity = { 0.f, 70.f,0.f };
		Vector3 AirCombo03Velocity = { 0.f, 70.f,0.f };
		Vector3 AirCombo04Velocity = { 0.f, -100.f,0.f };
	};
	enum class State : uint8
	{
		CombatWait = 0u,

		Run,
		RunEnd,

		JumpStart,
		JumpUp,
		Jump,
		JumpDown,
		JumpLanding,

		BasicCombo01,
		BasicCombo02,
		BasicCombo03,

		ComboEx01,

		ComboEx02Start,
		ComboEx02Loop,
		ComboEx02End,

		Dash,

		DashCombo,

		StandUpRolling ,

		StandBigBack,
		StandBigFront,
		StandBigLeft,
		StandBigRight,

		AirCombo01,
		AirCombo02,
		AirCombo03,
		AirCombo04,
		AirCombo04Landing,

		LeafAttackReady,
		LeafAttackStart,
		LeafAttackUp,
		LeafAttackDown,
		LeafAttackLanding,
	};
	void Initialize(const std::optional<Vector3>& Scale,
					const std::optional<Vector3>& Rotation,
					const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device)&;
	virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
	virtual std::optional<Engine::Object::SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
public:
	virtual void Event()&override;
	virtual void Update(const float DeltaTime)&;
	
	void Edit()&;

	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale)&override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;

	virtual std::function< SpawnReturnValue(const SpawnParam&)> 
		PrototypeEdit() & override;
private:
	struct FSMControlInformation
	{
		Engine::Transform* const MyTransform; 
		const Engine::Controller& _Controller;
		Engine::SkeletonMesh* const MySkeletonMesh;
		const float DeltaTime;
	};
	struct MoveControlInformation 
	{
		std::vector<Vector3> ControlDirections{};
	};
	void FSM(const float DeltaTime)&;
private:
	void CombatWaitTransition(const FSMControlInformation& FSMControlInfo)&;
	void CombatWaitState(     const FSMControlInformation& FSMControlInfo)&;
	void RunState(            const FSMControlInformation& FSMControlInfo)&;
	void RunTransition(const FSMControlInformation& FSMControlInfo,
						const std::optional<Player::MoveControlInformation>& MoveInfo)&;
	void RunEndState(         const FSMControlInformation& FSMControlInfo)&;
	void RunEndTransition(    const FSMControlInformation& FSMControlInfo)&;

	void JumpStartState       ( const FSMControlInformation& FSMControlInfo)&;
	void JumpStartTransition  ( const FSMControlInformation& FSMControlInfo)&;
	void JumpUpState          ( const FSMControlInformation& FSMControlInfo)&;
	void JumpUpTransition     ( const FSMControlInformation& FSMControlInfo)&;
	void JumpState            ( const FSMControlInformation& FSMControlInfo)&;
	void JumpTransition       ( const FSMControlInformation& FSMControlInfo)&;
	void JumpDownState        ( const FSMControlInformation& FSMControlInfo)&;
	void JumpDownTransition   ( const FSMControlInformation& FSMControlInfo)&;
	void JumpLandingState     ( const FSMControlInformation& FSMControlInfo)&;
	void JumpLandingTransition( const FSMControlInformation& FSMControlInfo)&;

	void BasicCombo01State(     const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo01Transition(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo02State(     const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo02Transition(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo03State(     const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo03Transition(const FSMControlInformation& FSMControlInfo)&;

	void AirCombo01State(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo01Transition(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo02State(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo02Transition(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo03State(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo03Transition(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04State(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04Transition(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04LandingState(const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04LandingTransition(const FSMControlInformation& FSMControlInfo)&;

	void ComboEx01State(    const FSMControlInformation& FSMControlInfo)&;
	void ComboEx01Transition(const FSMControlInformation& FSMControlInfo)&;

	void ComboEx02StartState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02StartTransition(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02LoopState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02LoopTransition(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02EndState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02EndTransition(const FSMControlInformation& FSMControlInfo)&;

	void DashState     (const FSMControlInformation& FSMControlInfo)&;
	void DashTransition(const FSMControlInformation& FSMControlInfo ,
					    const Player::MoveControlInformation& MoveControlInfo)&;
	
	void DashComboState (const FSMControlInformation& FSMControlInfo)&;
	void DashComboTransition (const FSMControlInformation& FSMControlInfo)&;

	void StandUpRollingState(const FSMControlInformation& FSMControlInfo )&; 
	void StandUpRollingTransition(const FSMControlInformation& FSMControlInfo ,
								const Player::MoveControlInformation& MoveControlInfo)&;

	void StandBigBackState(const FSMControlInformation& FSMControlInfo)&;
	void StandBigBackTransition(const FSMControlInformation& FSMControlInfo)&;
	void StandBigFrontState(const FSMControlInformation& FSMControlInfo)&;
	void StandBigFrontTransition(const FSMControlInformation& FSMControlInfo)&;
	void StandBigLeftState(const FSMControlInformation& FSMControlInfo)&;
	void StandBigLeftTransition(const FSMControlInformation& FSMControlInfo)&;
	void StandBigRightState(const FSMControlInformation& FSMControlInfo)&;
	void StandBigRightTransition(const FSMControlInformation& FSMControlInfo)&;

	void LeafAttackReadyState(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackReadyTransition(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackStartState(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackStartTransition(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackUpState(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackUpTransition(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackDownState(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackDownTransition(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackLandingState(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackLandingTransition(const FSMControlInformation& FSMControlInfo)&;
private:
	std::optional<Player::MoveControlInformation> 
		 CheckTheMoveableState  (const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheJumpableState  (const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheAttackableState(const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheLeafAttackableState(const FSMControlInformation& FSMControlInfo)&;
private:
	void MoveFromController(const FSMControlInformation& FSMControlInfo,
							const Player::MoveControlInformation& MoveControlInfo ,
							const float CurrentStateSpeed)&;

	bool IsSpeedInTheAir(const float YAxisVelocity)&;
private:
	void WeaponPut()&;
	void WeaponHand()&;
public:
	Vector3 CurrentMoveDirection{ 0.f,0.f,1.f };
	StateDuringSpeed StateableSpeed{};
private:
	Vector3 DashDirection{ 0,0,1 };
	bool bControl    {false};

	float PlayerMoveDirectionInterpolateAcceleration = 7.7f;
	Engine::ThirdPersonCamera* CurrentTPCamera{ nullptr };
	State   CurrentState{ Player::State::CombatWait};
	IDirect3DDevice9* Device{ nullptr };
};

