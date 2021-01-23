#pragma once
#include "RenderObject.h"

class TombStone final: public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& SpawnLocation  ,const Vector3& Rotation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device ,
							const Engine::RenderInterface::Group _Group)&;
	virtual void Event()&override;
	virtual void Render() & override;
	virtual void Update(const float DeltaTime)&;
	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale)&override;
private:
	uint32 _TestID = 0u;
	IDirect3DDevice9* Device{ nullptr };
};

