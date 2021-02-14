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
	enum class Mode : uint8 
	{
		NaviMesh=0u,
		Landscape
	};
private:
	void NaviMeshTool()&;
	void Landscape()&;
private:
	struct DecoratorOption
	{
		IDirect3DTexture9* Picture{};
		float Width;
		float Height;
	};
	std::map<std::wstring,DecoratorOption> DecoratorOpts{};

	Mode CurrentMode{ Mode::Landscape }; 
	const Vector3 MapScale{0.01f,0.01f,0.01f };
	const Vector3 MapRotation{0,0,0};
	const Vector3 MapLocation{0,0,0}; 
	const Matrix MapWorld = FMath::WorldMatrix(MapScale, MapRotation, MapLocation);

	uint32 NaviMeshCurrentSelectMarkeyKey{ 0u };
	uint32 NaviMeshCurrentSelectCellKey{ 0u };
	// 네비게이션 메쉬의 피킹시 편의를 위함.
	std::vector<PlaneInfo>  PickingPlanes{};
	Engine::NavigationMesh* _NaviMesh{ nullptr };
	int32 NavigationMeshModeSelect{ 0u };
	const uint32 MaxNavigationMeshMode{ 3u };
};

