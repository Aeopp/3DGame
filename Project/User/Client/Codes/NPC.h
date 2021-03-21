#pragma once
#include "Object.h"
#include "SkeletonMesh.h"

class NPC  final : public Engine::Object
{
public:
	using Super = Engine::Object;
	using MyType = NPC;

	enum class InteractionEvent : uint8
	{
		FirstEncounter,
		SecondEncounter,
	};
public:
	void Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	virtual std::shared_ptr<Engine::Object> GetCopyShared()& override;
	virtual std::optional<SpawnReturnValue>
		InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)& override;
	virtual std::function< SpawnReturnValue(const SpawnParam&)>
		PrototypeEdit()& override ;
	virtual void Event()&override;
	virtual void Update(const float DeltaTime)&;
	virtual void LateUpdate(const float DeltaTime)&;

	void Edit()&;

	virtual void HitNotify(Object* const Target,
		const Vector3 PushDir, const float CrossAreaScale) & override; 

	Vector3 ViewLocationOffset{ 0,15.929f, -5.310f};
private:
	IDirect3DDevice9* Device{ nullptr };
};

