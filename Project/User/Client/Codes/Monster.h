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

protected:
	struct FSMControlInformation
	{
		Engine::Transform* const MyTransform; 
		Engine::SkeletonMesh* const MySkeletonMesh;
		const float DeltaTime;
	};
	virtual void FSM(const float DeltaTime)&;
private:

public:
	Vector3 CurrentMoveDirection{ 0.f,0.f,1.f };
protected:
	float LandCheckHighRange = 7.f;
	const Engine::Cell* CurrentCell{nullptr};
	IDirect3DDevice9* Device{ nullptr };
};

