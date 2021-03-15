#pragma once
#include "Monster.h"
#include "Geometric.h"
#include "Bone.h"
#include <memory>

class Belatos final: public Monster 
{
public:
	using MyType = Belatos;
	using Super = Monster;
public:
	struct StateDuringSpeed
	{
		float Run = 5.f;
	};
	enum class State : uint8
	{
		Wait = 0u,
		Run,
		RunEnd,
		Skill1st,
		Skill2nd,
		Respawn ,
		RTStand ,
	};
	virtual void FSM(const float DeltaTime)& override;
	virtual void Edit()& override;
	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;
	virtual void Event() & override;
	virtual void Update(const float DeltaTime)&;
	virtual void LateUpdate(const float DeltaTime)&;
	virtual std::function< SpawnReturnValue(const SpawnParam&)>
				PrototypeEdit() & override;
	void PrototypeInitialize(IDirect3DDevice9* const Device) & ;

	void Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&;

	virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
	virtual std::optional<Engine::Object::SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
private:
	void WaitTransition(const FSMControlInformation& FSMControlInfo)&;
	void WaitState(const FSMControlInformation& FSMControlInfo)&;
	void RunState(const FSMControlInformation& FSMControlInfo)&;
	void RunTransition(const FSMControlInformation& FSMControlInfo)&;
	void RunEndState(const FSMControlInformation& FSMControlInfo)&;
	void RunEndTransition(const FSMControlInformation& FSMControlInfo)&;

	void Skill1stState(const FSMControlInformation& FSMControlInfo)&;
	void Skill1stTransition(const FSMControlInformation& FSMControlInfo)&;
	void Skill2ndState(const FSMControlInformation& FSMControlInfo)&;
	void Skill2ndTransition(const FSMControlInformation& FSMControlInfo)&;

	void RespawnState(const FSMControlInformation& FSMControlInfo)&;
	void RespawnTransition(const FSMControlInformation& FSMControlInfo)&;
	void RTStandState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandTransition(const FSMControlInformation& FSMControlInfo)&;
private:
	void WeaponAttackCollisionSweep(const FSMControlInformation& FSMControlInfo)&;
private:
	Vector3 WeaponLocalMin{};
	Vector3 WeaponLocalMax{};
	Engine::Bone* WeaponHandleBone{ nullptr };
	StateDuringSpeed StateableSpeed{};
	State   CurrentState{ Belatos::State::Wait };
};