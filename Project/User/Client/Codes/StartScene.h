#pragma once

#include "Scene.h"
#include <type_traits>
#include <numeric>
#include <filesystem>
#include "Vertexs.hpp"



class StartScene final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Event()& override;
	virtual void Update(const float DeltaTime) & override;
private:

};

