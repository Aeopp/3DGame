#pragma once

#include "Scene.h"
#include <type_traits>
#include <numeric>
#include <filesystem>
#include "Vertexs.hpp"
#include "NavigationMesh.h"


class Tool final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Event()& override;
	virtual void Update(const float DeltaTime) & override;
private:
	// 네비게이션 메쉬의 피킹시 편의를 위함.
	std::vector<PlaneInfo> PickingPlanes{};
	Engine::NavigationMesh* _NaviMesh{ nullptr };
	uint32 NavigationMeshModeSelect{ 0u };
	const uint32 MaxNavigationMeshMode{ 3u };
};

