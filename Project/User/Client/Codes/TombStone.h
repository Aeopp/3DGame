#pragma once
#include "RenderObject.h"

class TombStone final: public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device ,
							const Engine::RenderInterface::Group _Group)&;
	virtual void Render() & override;
	virtual void Update(const float DeltaTime)&;
private:
	IDirect3DDevice9* Device{ nullptr };
};

