#pragma once

#include "Scene.h"
#include <type_traits>
#include <array>
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
	enum class SpawnTransformItem :int32
	{
		CustomTransform=0,
		PickOvertheLandscape,
		InFrontOf,
	};
	std::map<std::wstring,DecoratorOption> DecoratorOpts{};
	float   SpawnEditScale = 1.f; 
	Vector3 SpawnEditLocation{ 0,0,0 };
	Vector3 SpawnEditRotation{ FMath::ToRadian(90.f),0,0 };
	SpawnTransformItem SpawnTransformComboSelectItem{ SpawnTransformItem::CustomTransform };
	std::array<const char*, 3u> SpawnTransformComboNames{
		"Custom Transform","Pick Over the Landscape","In front of"};

	float InfrontOfScale = 100.f;

	std::weak_ptr<typename Engine::Landscape::DecoInformation > CurEditDecoInstance{};

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

