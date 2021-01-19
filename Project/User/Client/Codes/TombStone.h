#pragma once
#include "Object.h"

class TombStone final: public Engine::Object
{
public:
	using Super = Engine::Object;
public:
	void Initialize()&;
	void PrototypeInitialize(IDirect3DDevice9*const Device)&;
public:
	virtual void Update(const float DeltaTime)&;
private:
	IDirect3DDevice9* Device{ nullptr };
};

