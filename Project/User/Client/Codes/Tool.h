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
	uint32 NaviMeshCurrentSelectMarkeyKey{ 0u };
	uint32 NaviMeshCurrentSelectCellKey{ 0u };
	// �׺���̼� �޽��� ��ŷ�� ���Ǹ� ����.
	std::vector<PlaneInfo> PickingPlanes{};
	Engine::NavigationMesh* _NaviMesh{ nullptr };
	int32 NavigationMeshModeSelect{ 0u };
	const uint32 MaxNavigationMeshMode{ 3u };
};
