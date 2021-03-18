#pragma once
#include <optional>
#include "Object.h"
#include "SkeletonMesh.h"
#include "ThirdPersonCamera.h"
#include "Cell.h"
#include "AnimEffect.h"

class Player final : public Engine::Object
{
public:
	using Super = Engine::Object;
public:
	struct LeafAttackInformation
	{
	public:
		void Reset( const Vector3& StartLocation, 
					const Vector3& DestLocation,
					const float Heighest,
					const float HeighestTime,
					const float t = 0.0f)&;
		std::optional<Vector3> Move(const float DeltaTime);
		Vector3 DestLocation = { 0,0,0 };
		Vector3 StartLocation = {0,0,0};
		// 최고점 높이
		float Heighest= 0.0f ;
		// 최고점 도달시간
		float HeighestTime = 0.0f ;
		float t;
	private:
		// 아래의 변수 들은 사용자의 매개변수에 의해 계산됨 
		Vector3 Velocity{ 0,0,0 };
		// 중력 가속도 
		float Gravity; 
		// 도착점 도달시간
		float ReachDestLocationTime = -1.f;
		// 최고점 - 시작점 (높이) 
		float ReachHeightTime;
		// 도착점 높이 
		float DestHeight;
	private:
		void PreCalculate()&;
	};
	struct StateDuringSpeed 
	{
		// 공중에 체공중이라고 판단하는 스피드 (애니메이션을 위함) 
		float InTheAirSpeed = 20.f;

		float Run = 55.f; 
		float ComboEx02 = 10.f;
		float Attack = 8.f; 
		float Jump = 32.2f;
		float Rolling = 55.f;
		float Dash = 110.f;
		float LeafReady = 20.f;

		Vector3 JumpVelocity = { 0.f,125.f,0.f };
		Vector3 AirCombo01Velocity = { 0.f, 80.f,0.f };
		Vector3 AirCombo02Velocity = { 0.f, 80.f,0.f };
		Vector3 AirCombo03Velocity = { 0.f, 80.f,0.f };
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
	virtual void LateUpdate(const float DeltaTime)&;

	void Edit()&;

	virtual void Hit(Object* const Target, const Vector3 PushDir,
		const  float CrossAreaScale) & override;

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
	struct AttackForceInformation
	{
		float BasicCombo = 0.1f;
		float BasicComboSmall = 0.2f;
		float BasicComboSmallJump = 55.f;
		float Ex01firstCombo = 0.5f;
		float Ex01firstComboJump = 65.f;

		float Ex01SecondCombo = 1.f;
		float Ex01SecondComboJump = 78.f;

		float Ex02Start = 0.1f;
		float Ex02StartJump = 0.0f;

		float Ex02Loop = 0.1f;
		float Ex02LoopJump = 0.0f;

		float Ex02End = 0.1f;
		float Ex02EndJump = 45.f;

		float Dash = 1.f;
		float DashComboJump= 70.f;

		float Air01 = 0.3f;
		float Air01Jump = 45.f;

		float Air02 = 0.3f;
		float Air02Jump = 45.f;

		float Air03 = 0.3f;
		float Air03Jump = 45.f;

		float Air04 = 0.3f;
		float Air04Jump = 45.f;
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
	void AirCombo02State(            const FSMControlInformation& FSMControlInfo)&;
	void AirCombo02Transition(       const FSMControlInformation& FSMControlInfo)&;
	void AirCombo03State(            const FSMControlInformation& FSMControlInfo)&;
	void AirCombo03Transition(       const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04State(            const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04Transition(       const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04LandingState(     const FSMControlInformation& FSMControlInfo)&;
	void AirCombo04LandingTransition(const FSMControlInformation& FSMControlInfo)&;

	void ComboEx01State(    const FSMControlInformation& FSMControlInfo)&;
	void ComboEx01Transition(const FSMControlInformation& FSMControlInfo)&;

	void ComboEx02StartState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02StartTransition(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02LoopState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02LoopTransition(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02EndState(const FSMControlInformation& FSMControlInfo)&;
	void ComboEx02EndTransition(const FSMControlInformation& FSMControlInfo)&;

	void DashState     (      const FSMControlInformation& FSMControlInfo)&;
	void DashTransition(      const FSMControlInformation& FSMControlInfo ,
					          const Player::MoveControlInformation& MoveControlInfo)&;
	
	void DashComboState (     const FSMControlInformation& FSMControlInfo)&;
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

	//C1 0.08 ~0.13
	//C20.14~0.18
	//C3 0.11~0.26o
	//Ex01 0.11 0.26
	//Ex02 Start o0.3~1.0
	//Ex02 end 0.08 0.145
	//Dashcombo 0.1 0.2
	//AC1 0.16 0.235
	//ac2 0.18 0.35
	//ac3 0.06 0.23
	//ac4 0.3 1.0

private:
	std::optional<Player::MoveControlInformation> 
		 CheckTheMoveableState  (const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheJumpableState  (const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheAttackableState(const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheLeafAttackableState(const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheLandingStatable(const float CurLocationY, const float CurGroundY)&;
private:
	void MoveFromController(const FSMControlInformation& FSMControlInfo,
							const Player::MoveControlInformation& MoveControlInfo ,
							const float CurrentStateSpeed)&;

	bool IsSpeedInTheAir(const float YAxisVelocity)&;
private:
	void WeaponPutDissolveStart()&;
	void WeaponAcquisition()&;
	void WeaponPut()&;
	void WeaponHand()&;
private:
	class PlayerWeapon* const  GetWeapon()const &;

public:
	bool bInvincibility = false;
	Vector3 CurrentMoveDirection{ 0.f,0.f,1.f };
	StateDuringSpeed StateableSpeed{};
	Engine::ThirdPersonCamera::TargetInformation PlayerTargetInfo{};
	Engine::ThirdPersonCamera* CurrentTPCamera{ nullptr };
private:
	class Engine::AnimEffect* _BasicCombo01{ nullptr };

	AttackForceInformation _AttackForce{};
	float WeaponDissolveTime = -1.f / 2.f;
	Vector3 NPCInteractionLocationOffset{ -7.080f,14.159f,3.540f};
	Vector3 PlayerCameraTargetLocationOffset{ 0.f,20.f,0.f };
	bool bNPCInteraction = false;
	bool bWeaponAcquisition = false;
	float LandCheckHighRange = 7.f;
	const Engine::Cell* CurrentCell{nullptr};
	LeafAttackInformation _LeafAttackInfo{};
	Vector3 DashDirection{ 0,0,1 };
	bool bControl    {false};
	float PlayerMoveDirectionInterpolateAcceleration = 7.7f;
	class PlayerWeapon* CurrentWeapon{ nullptr };
	State   CurrentState{ Player::State::CombatWait};
	IDirect3DDevice9* Device{ nullptr };
};

