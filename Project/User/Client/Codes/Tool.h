#pragma once

#include "Scene.h"
#include <type_traits>
#include <memory>
#include <numeric>
#include <filesystem>
#include "Landscape.h"
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
	virtual void Render() &override;
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

	
	std::weak_ptr<typename Engine::Landscape::DecoInformation > CurEditTransform{};

	Mode CurrentMode{ Mode::Landscape }; 
	const float  MapScale = 0.01f;  
	const Vector3 MapRotation{3.14f/2.f,0.f,0.f };
	const Vector3 MapLocation{0,0,0}; 

	ID3DXLine* LinearSpace{};
	uint32 NaviMeshCurrentSelectMarkeyKey{ 0u };
	uint32 NaviMeshCurrentSelectCellKey{ 0u };
	// 네비게이션 메쉬의 피킹시 편의를 위함.
	std::vector<PlaneInfo>  PickingPlanes{};
	Engine::NavigationMesh* _NaviMesh{ nullptr };
	int32 NavigationMeshModeSelect{ 0u };
	const uint32 MaxNavigationMeshMode{ 3u };
};

