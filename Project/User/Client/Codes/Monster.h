#pragma once
#include <optional>
#include "Object.h"
#include "SkeletonMesh.h"
#include "Cell.h"

class Monster  : public Engine::Object
{
public:
	using Super = Engine::Object;
public:
	void MonsterInitialize(const std::optional<Vector3>& Scale,
					const std::optional<Vector3>& Rotation,
					const Vector3& SpawnLocation)&;
	void MonsterPrototypeInitialize(IDirect3DDevice9*const Device)&;
public:
	virtual std::shared_ptr<Engine::Object> GetCopyShared()& override;
	virtual std::optional<SpawnReturnValue>
		InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)& override;
	virtual std::function< SpawnReturnValue(const SpawnParam&)>
		PrototypeEdit()& override ;
	virtual void Event()&override;
	virtual void Update(const float DeltaTime)&;
	virtual void LateUpdate(const float DeltaTime)&;

	virtual void Edit()&;

	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale)&override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;

	bool  IsInvincibility()const& { return CurInvincibilityTime >0.1f; };
	float TakeDamage(const float Damage)&;
	bool  IsAttackRange(const Vector3& TargetLocation);;
protected:
	struct FSMControlInformation
	{
		class Player* const _Player{ nullptr };
		Engine::Transform* const MyTransform; 
		Engine::SkeletonMesh* const MySkeletonMesh;
		const float DeltaTime;
	};
	struct Status
	{
		float HP = 10000.f;
	};
	virtual void FSM(const float DeltaTime)&;

	void LockingToWardsFromDirection(Vector3 Direction)&;
public:
	bool  bBackHit  =false ;
	bool  bFrontHit =false ;
	bool  bLeftHit  =false ;
	bool  bRightHit =false ;
	Vector3 CurrentMoveDirection{ 0.f,0.f,1.f };
protected:
	bool CheckTheLandingStatable(const float CurLocationY, const float CurGroundY)&;
protected:
	float AirDamageToForceFactor = 0.12f;
	float DamageToForceFactor = 0.12f;
	float AttackRange = 23.f;
	float ResetInvincibilityTime = 0.1f;
	float CurInvincibilityTime = ResetInvincibilityTime;
	Status _Status{}; 
	float LandCheckHighRange = 7.f;
	const Engine::Cell* CurrentCell{nullptr};
	IDirect3DDevice9* Device{ nullptr };
};

