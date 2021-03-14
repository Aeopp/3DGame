#pragma once
#include "Monster.h"


class Belatos final: public Monster 
{
public:
	using MyType = Belatos;
	using Super = Monster;
public:
	struct StateDuringSpeed
	{
		float Run = 55.f;
	};
	enum class State : uint8
	{
		Wait = 0u,
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
private:
	StateDuringSpeed StateableSpeed{};
	State   CurrentState{ Belatos::State::Wait };
};