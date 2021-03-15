#pragma once

#include "Scene.h"
#include <type_traits>
#include <numeric>
#include <filesystem>
#include "Vertexs.hpp"
#include "ShaderFx.h"



class StartScene final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Event()& override;
	virtual void Update(const float DeltaTime) & override;
	virtual void Render() & override;
private:
	bool bGameStart = false;
	IDirect3DVertexBuffer9* LogoVtxBuf{ nullptr };
	IDirect3DTexture9* LogoTexture{ nullptr };
	Engine::ShaderFx* Sample{nullptr};
};

