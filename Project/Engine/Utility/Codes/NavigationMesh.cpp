#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "Cell.h"

void Engine::NavigationMesh::InsertPoint(const Vector3 Point)&
{
	CurrentPickPoints.push_back(Point);
	if (CurrentPickPoints.size() >= 3u)
	{
		const uint32 CellContainerIdx = CellContainer.size();
		auto _InsertCell =  std::make_shared<Cell>();
		_InsertCell->Initialize(CellContainerIdx,
								CurrentPickPoints[0],
								CurrentPickPoints[1],
								CurrentPickPoints[2]);
		CellContainer.push_back(std::move(_InsertCell));
	}
}

void Engine::NavigationMesh::Initialize(IDirect3DDevice9* Device)&
{
	this->Device = Device;
	// 여기서 파일 로딩...
}

void Engine::NavigationMesh::Render(IDirect3DDevice9* const Device)&
{
	for (auto& CurCell : CellContainer)
	{
		CurCell->Render(Device);
	}
}

std::optional<std::pair<Vector3,uint32>> 
Engine::NavigationMesh::MoveOnNavigation(
	const Vector3 TargetDirection,
	const Vector3 TargetLocation, 
	const uint32 CellIndex)&
{
	const Vector3 EndLocation = TargetLocation + TargetDirection;
	const auto  [_Result ,NewIndex ] = 
		CellContainer[CellIndex]->Compare(EndLocation, CellIndex);

	switch (_Result)
	{
	case Engine::Cell::CompareType::Moving:
		return { { TargetDirection,NewIndex }  };
		break; 
	case Engine::Cell::CompareType::Stop:
		return std::nullopt;
		break; 
	default:
		return std::nullopt;
		break;
	}
}
