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
	void CombatWaitState(const FSMControlInformation& FSMControlInfo)&;
	void RunState(const FSMControlInformation& FSMControlInfo)&;
	void RunTransition(const FSMControlInformation& FSMControlInfo)&;
	void RunEndState(const FSMControlInformation& FSMControlInfo)&;
	void RunEndTransition(const FSMControlInformation& FSMControlInfo)&;

	void JumpStartState       (const FSMControlInformation& FSMControlInfo)&;
	void JumpStartTransition  (const FSMControlInformation& FSMControlInfo)&;
	void JumpUpState          (const FSMControlInformation& FSMControlInfo)&;
	void JumpUpTransition     (const FSMControlInformation& FSMControlInfo)&;
	void JumpState            (const FSMControlInformation& FSMControlInfo)&;
	void JumpTransition       (const FSMControlInformation& FSMControlInfo)&;
	void JumpDownState        (const FSMControlInformation& FSMControlInfo)&;
	void JumpDownTransition   (const FSMControlInformation& FSMControlInfo)&;
	void JumpLandingState     (const FSMControlInformation& FSMControlInfo)&;
	void JumpLandingTransition(const FSMControlInformation& FSMControlInfo)&;

	void BasicCombo01State(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo01Transition(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo02State(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo02Transition(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo03State(const FSMControlInformation& FSMControlInfo)&;
	void BasicCombo03Transition(const FSMControlInformation& FSMControlInfo)&;
private:
	std::optional<Player::MoveControlInformation> CheckTheMoveableState(const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheJumpableState(const FSMControlInformation& FSMControlInfo)&;
	bool CheckTheAttackableState(const FSMControlInformation& FSMControlInfo)&;
private:
	void MoveFromController(const FSMControlInformation& FSMControlInfo,
							const Player::MoveControlInformation& MoveControlInfo)&;
private:
	float PlayerMoveDirectionInterpolateAcceleration = 7.f;
	Engine::ThirdPersonCamera* CurrentTPCamera{ nullptr };
	Vector3 CurrentMoveDirection{ 0.f ,  0.f  , 1.f };
	float RunSpeed = 333.f;
	State CurrentState{ Player::State::CombatWait};
	IDirect3DDevice9* Device{ nullptr };
};

