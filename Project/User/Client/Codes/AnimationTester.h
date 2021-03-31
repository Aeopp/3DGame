#pragma once
#include "Monster.h"
#include "Geometric.h"
#include "Bone.h"
#include <memory>
#include "ThirdPersonCamera.h"

class AnimationTester final  : public Engine::Object
{
public:
	using Super = Engine::Object;
	using MyType = AnimationTester;
public:
	void Initialize(const std::optional<Vector3>& Scale,
		const std::optional<Vector3>& Rotation,
		const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
	virtual std::optional<Engine::Object::SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
public:
	virtual void Event() & override;
	virtual void Update(const float DeltaTime)&;

	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;

	virtual std::function< SpawnReturnValue(const SpawnParam&)>
		PrototypeEdit() & override;
private:
	std::array<char, 256u> EditBoneNameBuf{};
	std::array<char, 256u> EditChangeModelBuf{};

	Engine::Object* _AttackTarget{ nullptr };
	IDirect3DDevice9* Device{ nullptr };
};