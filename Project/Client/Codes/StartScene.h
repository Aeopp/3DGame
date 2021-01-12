#pragma once
#include "Scene.h"

class StartScene final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Update(const float DeltaTime) & override;
private:
};

