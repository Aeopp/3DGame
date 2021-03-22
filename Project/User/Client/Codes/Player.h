#pragma once
#include <optional>
#include "Object.h"
#include "SkeletonMesh.h"
#include "ThirdPersonCamera.h"
#include "Cell.h"
#include "AnimEffect.h"
#include <memory>
#include "UI.h"

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
		static inline float LeafAttackAxisDelta = 1.f;

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
		float Rolling = 120.f;
		float Dash = 140.f;
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
	void CreatePlayerSkillUI()&;
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
		float BasicComboEx03_1 = 0.2f;
		float BasicComboEx03_1_Jump = 55.f;

		float BasicComboEx03_2 = 0.2f;
		float BasicComboEx03_2_Jump = 100.f;

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

	void AirCombo01State(     const FSMControlInformation& FSMControlInfo)&;
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
	void LeafReadyCameraUpdate(const FSMControlInformation& FSMControlInfo)&;
	void LeafAttackCameraUpdate(const FSMControlInformation& FSMControlInfo)&;
private:
	void SwordEffectPlay(class Engine::AnimEffect* _AnimEffect , const FSMControlInformation& FSMControlInfo ,
		const Vector3 RotationOffset, const Vector3 LocationOffset = { 0,0,0 },
		const float TimeAcceleration=0.9f ,
		const bool bLoop=false , 
		const bool bLinearAlpha=true ,
		const float AlphaFactor=1.f,
		const Vector2& GradientUVOffsetFactor = { 1.f,1.f } , 
		const std::wstring& DiffuseMap = L"iceImage",
		const std::wstring& PatternMap = L"type_37",
		const std::wstring& AddColorMap = L"T_HFH_Basic_Attack_Front",
		const std::wstring& UVDistorMap = L"Cartoon_Distortion_0008")&;
	void SwordCameraShake(const float Force = 4.f, const float Duration = 0.2f)&;
private:
	class PlayerWeapon* const  GetWeapon()const &;
public:
	bool bInvincibility = false;
	Vector3 CurrentMoveDirection{ 0.f,0.f,1.f };
	StateDuringSpeed StateableSpeed{};
	Engine::ThirdPersonCamera::TargetInformation PlayerTargetInfo{};
	Engine::ThirdPersonCamera* CurrentTPCamera{ nullptr };
private:
	std::weak_ptr<Engine::UI>    DoubleSlashSlot{};
	std::weak_ptr<Engine::UI>    DoubleSlashIcon{};

	std::weak_ptr<Engine::UI>    KarmaIcon{};
	std::weak_ptr<Engine::UI>    KarmaSlot{};

	std::weak_ptr<Engine::UI>    LeapAttackSlot{};
	std::weak_ptr<Engine::UI>    LeapAttackIcon{};

	std::weak_ptr<Engine::UI>    AvoidSlot{};
	std::weak_ptr<Engine::UI>    AvoidIcon{};

	std::weak_ptr<Engine::UI>    OutRangeSlot{};
	std::weak_ptr<Engine::UI>    OutRangeIcon{};
	std::weak_ptr<Engine::UI>    RockBreakSlot{};
	std::weak_ptr<Engine::UI>    RockBreakIcon{};

	std::weak_ptr<Engine::UI> RockShotIcon{};
	std::weak_ptr<Engine::UI> RockShotSlot{};

	std::weak_ptr<Engine::UI> PlayerKarmaInfoGUI{};
	std::weak_ptr<Engine::UI> CenterLineQuad{};

	std::weak_ptr<Engine::UI> ScreenBloodQuad{};
	std::weak_ptr<Engine::UI> MouseUI{};

	static constexpr float CenterLineQuadAlphaFactorAcceleration = 2.f;
	static constexpr float ScreenBloodQuadAlphaFactorAcceleration = 2.f;
private:
	
	static const inline float StandUpRollingCoolTime = 1.f; 
	
	float CurrentStandUpRollingCoolTime = StandUpRollingCoolTime;
private:
	const float ContinuousAttackCorrectionTime = 0.1f;
	float CurrentContinuousAttackCorrectionTime = ContinuousAttackCorrectionTime;

	bool bAura = false;
	class Engine::AnimEffect* _BasicCombo01{ nullptr };
	class Engine::AnimEffect* _BasicCombo02{ nullptr };
	class Engine::AnimEffect* _BasicCombo03_1{ nullptr };
	class Engine::AnimEffect* _BasicCombo03_2{ nullptr };
	class Engine::AnimEffect* _Ex01_1{ nullptr };
	class Engine::AnimEffect* _Ex01_2{ nullptr };
	class Engine::AnimEffect* _Ex02_Start01{ nullptr };
	class Engine::AnimEffect* _Ex02_Loop{ nullptr };
	class Engine::AnimEffect* _Ex02_End{ nullptr };

	class Engine::AnimEffect* AirCombo01{ nullptr };
	class Engine::AnimEffect* AirCombo02{ nullptr };
	class Engine::AnimEffect* AirCombo03{ nullptr };
	class Engine::AnimEffect* AirCombo04{ nullptr };

	class Engine::AnimEffect* LeapAttack01{ nullptr };

	
	AttackForceInformation _AttackForce{};
	float WeaponDissolveTime = -1.f / 2.f;
	Vector3 NPCInteractionLocationOffset{ -7.080f,14.159f,3.540f};
	Vector3 PlayerCameraTargetLocationOffset{ 0.f,13.f,0.f };
	
	bool  bWeaponAcquisition = false;
	float LandCheckHighRange = 7.f;
	const Engine::Cell* CurrentCell{nullptr};
	LeafAttackInformation _LeafAttackInfo{};
	Vector3 DashDirection{ 0,0,1 };
	bool bControl    {false};
	float PlayerMoveDirectionInterpolateAcceleration = 7.7f;
	class PlayerWeapon* CurrentWeapon{ nullptr };
	State   CurrentState{ Player::State::CombatWait};
	IDirect3DDevice9* Device{ nullptr };
	float HP = 100.f;
};

