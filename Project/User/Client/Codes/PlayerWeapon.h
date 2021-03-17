#pragma once
#include "Object.h"
#include <optional>

class PlayerWeapon final: public Engine::Object
{
public:
	using Super = Engine::Object;
public:
	void Initialize(const std::optional< Vector3>& Scale,
					const std::optional< Vector3>& Rotation,
					const Vector3& SpawnLocation )&;
	void PrototypeInitialize(IDirect3DDevice9*const Device)&;
	virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
	virtual
		std::optional<Engine::Object::SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
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

	auto GetDamage()const& { return DamageRange;  };
	void SetDamage(const std::pair<float,float>& DamageRange )&   { this->DamageRange = DamageRange;  };

	void DissolveStart(const float SliceAmountSpeed,
					   const float SliceAmountStart)&;

	void StartAttack(Engine::Object* const AttackOwner, const float ForcePitchRad=0.0f)&;
	void EndAttack(Engine::Object*const AttackOwner)&;
	Engine::Object*const GetAttackOwner() const& { return AttackOwner; };

	float ForcePitchRad = 0.0f;
private:
	Engine::Object* AttackOwner{ nullptr };
	float SliceAmountSpeed = 1.f;
	std::pair<float, float >DamageRange{ 700.f, 1000.f };
	IDirect3DDevice9* Device{ nullptr };
};

