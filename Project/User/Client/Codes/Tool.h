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
	void NaviMeshTool()&;
private:
	const Vector3 MapScale{33,77,44};
	const Vector3 MapRotation{FMath::ToRadian(90.f) , FMath::ToRadian(45.f),44.f};
	const Vector3 MapLocation{7,4,100}; 
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);

	uint32 NaviMeshCurrentSelectMarkeyKey{ 0u };
	uint32 NaviMeshCurrentSelectCellKey{ 0u };
	// 네비게이션 메쉬의 피킹시 편의를 위함.
	std::vector<PlaneInfo> PickingPlanes{};
	Engine::NavigationMesh* _NaviMesh{ nullptr };
	int32 NavigationMeshModeSelect{ 0u };
	const uint32 MaxNavigationMeshMode{ 3u };
};

