#pragma once
#include "RenderObject.h"

class TombStone final: public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& Scale,
					const Vector3& Rotation,
					const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device ,
							const Engine::RenderInterface::Group _Group)&;
	virtual void Event()&override;
	virtual void Render(const Matrix& View, const Matrix& Projection,
		const Vector4& CameraLocation) & override;
	virtual void Update(const float DeltaTime)&;

	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale)&override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;

	virtual void PrototypeEdit() & override;
private:
	IDirect3DDevice9* Device{ nullptr };
};

