#pragma once

#include "Scene.h"
#include <type_traits>
#include <array>
#include <memory>
#include <numeric>
#include <filesystem>
#include "Landscape.h"

class ObjectEdit final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Event()& override;
	virtual void Update(const float DeltaTime) & override;
	virtual void Render() &override;
private:
	void CaptureObjectLoad(const std::filesystem::path& FullPath)&;
	void CaptureCurrentObjects()&;
	void DecoratorSave(Engine::Landscape& Landscape)const&;
	void DecoratorLoad(Engine::Landscape& Landscape)&;
private:
	Engine::Object::SpawnParam CurSpawnParam{};
};

