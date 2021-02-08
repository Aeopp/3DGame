#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>
#include <set>

static uint32 MarkerKey{ 1u };
static uint32 CellKey{ 1u };

void Engine::NavigationMesh::CellNeighborLink()&
{
	for (auto&[CellKey,_Cell] : CellContainer)
	{
		_Cell->Neighbors.clear();
		for (const uint32 CurCellMarkerKey : _Cell->MarkerKeys)
		{
			for (const uint32 NeighborTargetCellKey :
				CurrentMarkers.find(CurCellMarkerKey)->second->SharedCellKeys)
			{
				Cell* NeighborTargetPtr = CellContainer.find(NeighborTargetCellKey)->second.get();

				// 마커키에는 마커를 공유하는 셀들이 전부 다 포함되므로
				// 이웃과 해당 셀이 같지 않을때만 이웃으로 지정.
				if (NeighborTargetPtr != _Cell.get())
				{
					_Cell->Neighbors.push_back(NeighborTargetPtr);
				}
			}
		}
	}
}

void Engine::NavigationMesh::Save(const std::filesystem::path SavePath) const&
{
	std::wofstream Os{ SavePath };
	for (const auto&  [CellKey,SaveCell] : CellContainer)
	{
		Os << L"Cell : " 
			<< L"Location A[" << SaveCell->PointA.x << L" " << SaveCell->PointA.y << L" " << SaveCell->PointA.z << "]"
			<< L"  B[" << SaveCell->PointB.x << L" " << SaveCell->PointB.y << L" " << SaveCell->PointB.z << "]"
			<< L"  C[" << SaveCell->PointC.x << L" " << SaveCell->PointC.y << L" " << SaveCell->PointC.z << "]" << std::endl;
	}
}

void Engine::NavigationMesh::Load(const std::filesystem::path SavePath)&
{

}

void Engine::NavigationMesh::DebugLog()&
{
	ImGui::Begin("NavigationMesh Information");
	if (auto iter = CellContainer.find(CurSelectCellKey);
		iter != std::end( CellContainer ))
	{
		ImGui::Separator();
		ImGui::Text("Select Cell Information"); 
		ImGui::Text("Cell Index : %d ", CurSelectCellKey);
		ImGui::Text("Marker Index..."); 

		auto SelectCell = iter->second;
		for (const uint32 MarkeyKey : SelectCell->MarkerKeys)
		{
			ImGui::Text("%d ", MarkeyKey);
			ImGui::SameLine();
		}
		ImGui::Text("Neighbor Keys... :");
		ImGui::SameLine();
		std::set<uint32> FilterAfterNeighborKeys{};
		for (const uint32 MarkeyKey : SelectCell->MarkerKeys)
		{
			if (auto MarkeyIter = CurrentMarkers.find(MarkeyKey);
				MarkeyIter != std::end(CurrentMarkers))
			{
				for (const uint32 NeighborKey : MarkeyIter->second->SharedCellKeys)
				{
					FilterAfterNeighborKeys.insert(NeighborKey);
				}
			}
		}
		FilterAfterNeighborKeys.erase(CurSelectCellKey);
		for (const uint32 FilterAfterNeighborKey : FilterAfterNeighborKeys)
		{
			ImGui::Text("%d ", FilterAfterNeighborKey);
			ImGui::SameLine();
		}
		ImGui::Separator();
	}

	ImGui::Separator();
	if (auto iter = CurrentMarkers.find(CurSelectMarkerKey);
		iter != std::end(CurrentMarkers))
	{
		auto SelectMarker = iter->second;
		ImGui::Text("Select Marker Information");
		ImGui::Text("Marker Index : %d ", CurSelectMarkerKey);
		ImGui::Text("Marker Point Shared Cell"); ImGui::SameLine();
		for (const uint32 MarkerPointSharedCell : SelectMarker->SharedCellKeys)
		{
			ImGui::Text("%d ", MarkerPointSharedCell);
			ImGui::SameLine();
		}
		ImGui::Separator();
	}
	ImGui::End();
}

void Engine::NavigationMesh::EraseCellFromRay(const Ray WorldRay)&
{
	for (auto iter = std::begin(CellContainer); 
			iter!= std::end(CellContainer) ;)
	{
		auto  [ CellKey,CurCell ] = *iter;
		const PlaneInfo CurPlane = 
			PlaneInfo::Make({ CurCell->PointA  ,CurCell->PointB, CurCell->PointC});
		float t; 
		Vector3 IntersectPoint;
		if (FMath::IsTriangleToRay(CurPlane, WorldRay, t, IntersectPoint))
		{
			// 셀과 연결된 마커들에게 삭제를 전파.
			auto DeleteCell = CurCell;
			for (const uint32 DeleteMarkeyKey : DeleteCell->MarkerKeys)
			{
				auto MarkerIter = CurrentMarkers.find(DeleteMarkeyKey);
				if (MarkerIter != std::end(CurrentMarkers))
				{
					// 셀 삭제시 해당 셀만 참조하고있던 마커라면 마커를 삭제
					// 그게 아니라면 마커에서 셀의 인덱스만 삭제.
					if (MarkerIter->second->SharedCellKeys.size() <= 1u)
					{
						CurrentMarkers.erase(MarkerIter);
					}
					else
					{
						MarkerIter->second->SharedCellKeys.erase(CellKey);
					}
				}
			}
			iter = CellContainer.erase(iter);
			continue;
		}
		else
		{
			++iter;
		}
	}
}

void Engine::NavigationMesh::MarkerMove(const uint32 MarkerKey, const Vector3 Vec)&
{
	if (MarkerKey == 0u)return;
	auto iter = CurrentMarkers.find(MarkerKey);
	if (iter == std::end(CurrentMarkers))return;

	auto TargetMarker  = iter->second;
	const Vector3 PrevPoint = TargetMarker->_Sphere.Center;
	const Vector3 NewPoint = TargetMarker->_Sphere.Center  += Vec;

	for (const uint32 CellKey : TargetMarker->SharedCellKeys)
	{
		auto Celliter = CellContainer.find(CellKey);
		if (Celliter != std::end(CellContainer))
		{
			auto   ResetPointCell  = Celliter->second;
			std::array <std::reference_wrapper<Vector3>, 3u>
				RefResetPoints{ ResetPointCell->PointA  ,  ResetPointCell->PointB ,ResetPointCell->PointC };

			for (auto& RefPoint : RefResetPoints)
			{
				if (FMath::Length( RefPoint.get() -  PrevPoint) <0.1f)
				{
					RefPoint.get() = NewPoint;
				}
			}

			ResetPointCell->ReCalculateSegment2D();
		}
	}
}

uint32 Engine::NavigationMesh::InsertPointFromMarkers(const Ray WorldRay)&
{
	for (auto& [MarkerKey, CurTargetMarker] : CurrentMarkers)
	{
		float t0, t1; Vector3 IntersectPt;
		const Sphere TargetSphere = CurTargetMarker->_Sphere;
		if (FMath::IsRayToSphere(WorldRay, TargetSphere, t0, t1, IntersectPt))
		{
			// 마커에 점을 피킹하고 삼각형을 이어야 하나 동일한 마커에 다시 피킹한 상황.
			for(const auto& PickPoint : CurrentPickPoints) 
			{
				if (PickPoint.first==MarkerKey)
				{
					return CurSelectMarkerKey=MarkerKey;
				}
			}

			CurrentPickPoints.push_back({ MarkerKey,TargetSphere.Center });

			if (CurrentPickPoints.size() >= 3u)
			{
				auto _InsertCell = std::make_shared<Cell>();

				_InsertCell->Initialize(this,
					CurrentPickPoints[0].second,
					CurrentPickPoints[1].second,
					CurrentPickPoints[2].second,
					Device,
					{   CurrentPickPoints[0].first ,
						CurrentPickPoints[1].first,
						CurrentPickPoints[2].first }
				);
				CurrentPickPoints.clear();
				const uint32 CurrentCellKey = CellKey++;  
				CellContainer.insert({ CurrentCellKey ,std::move(_InsertCell) });
				CurTargetMarker->SharedCellKeys.insert(CurrentCellKey);
			}

			return CurSelectMarkerKey=MarkerKey;
		}
	}
	return 0u;
}

uint32 Engine::NavigationMesh::SelectMarkerFromRay(const Ray WorldRay)&
{
	for (auto& [MarkerKey, CurTargetMarker] : CurrentMarkers)
	{
		float t0, t1; Vector3 IntersectPt;
		const Sphere TargetSphere = CurTargetMarker->_Sphere;
		if (FMath::IsRayToSphere(WorldRay, TargetSphere, t0, t1, IntersectPt))
		{
			return  CurSelectMarkerKey=MarkerKey;
		}
	}
	return 0u;
}

uint32 Engine::NavigationMesh::SelectCellFromRay(const Ray WorldRay)&
{
	for (const auto& [CellKey,_Cell] : CellContainer)
	{
		float t;
		Vector3 IntersectPoint; 
		if (FMath::IsTriangleToRay(_Cell->_Plane, WorldRay, t, IntersectPoint))
		{
			return CurSelectCellKey = CellKey; 
		}
	}
	return  0u;
}

uint32 Engine::NavigationMesh::InsertPoint(const Vector3 Point)&
{
	auto _Marker = std::make_shared<Marker>();
	_Marker->_Sphere.Radius = 1.f;
	_Marker->_Sphere.Center = Point;
	const uint32 CurrentMarkeyKey = MarkerKey++;
	CurrentMarkers.insert({ CurrentMarkeyKey,_Marker});
	CurrentPickPoints.push_back({ CurrentMarkeyKey, Point });

	if (CurrentPickPoints.size() >= 3u)
	{
		auto _InsertCell = std::make_shared<Cell>();

		_InsertCell->Initialize(this,
				CurrentPickPoints[0].second,
				CurrentPickPoints[1].second,
				CurrentPickPoints[2].second,
				Device,
			{   CurrentPickPoints[0].first ,
				CurrentPickPoints[1].first,
				CurrentPickPoints[2].first }
		);
		
		const uint32 CurrentCellKey = CellKey++;
		CellContainer.insert({ CurrentCellKey ,std::move(_InsertCell) });
		for (const auto& [ PickKey, PickPoint] : CurrentPickPoints)
		{
			CurrentMarkers.find(PickKey)->second->SharedCellKeys.insert(CurrentCellKey);
		}
		CurrentPickPoints.clear();
	}

	return CurrentMarkeyKey;
}

void Engine::NavigationMesh::Initialize(IDirect3DDevice9* Device)&
{
	this->Device = Device;

	Device->CreateVertexBuffer(sizeof(Vertex::LocationColor) * 3u*3000u, D3DUSAGE_DYNAMIC, Vertex::LocationColor::FVF,D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
	ResourceSystem::Instance->Insert<IDirect3DVertexBuffer9>(L"NaviMeshDebugVertexBuffer", VertexBuffer);
	// 여기서 파일 로딩...
}

void Engine::NavigationMesh::Render(IDirect3DDevice9* const Device)&
{
	if (Engine::Global::bDebugMode)
	{
		for (auto& [CellKey , CurCell ] : CellContainer)
		{
			CurCell->Render(Device);
		}
		Vertex::LocationColor* VtxBufPtr{ nullptr };
		VertexBuffer->Lock(0u,
		sizeof(Vertex::LocationColor) * 3u * CellContainer.size(), reinterpret_cast<void**>			(&VtxBufPtr),NULL);
		
		uint32 Idx = 0u;
		for (const auto& [CellKey, _Cell] : CellContainer)
		{
			for (uint32 i = 0; i < 3; ++i)
			{
				const uint32 TargetBufferIdx =Idx * 3u + i;
				if (i == 0)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointA;
				if (i == 1)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointB;
				if (i == 2)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointC;

				VtxBufPtr[TargetBufferIdx].Diffuse = D3DCOLOR_ARGB(255, 165, 171, 255);
			}
			++Idx;
		}

		VertexBuffer->Unlock();
		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		const Matrix Identity = FMath::Identity(); 
		Device->SetTransform(D3DTS_WORLD, &Identity); 
		Device->SetStreamSource(0, VertexBuffer, 0u, sizeof(Vertex::LocationColor));
		Device->SetFVF(Vertex::LocationColor::FVF);
		Device->SetVertexShader(nullptr);
		Device->SetPixelShader(nullptr);
		Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0u, CellContainer.size());
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		auto DebugSphereMesh = ResourceSystem::Instance->Get<ID3DXMesh>(L"SphereMesh");
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		for (auto& [MarkeyKey,DrawMarkerDebugSphere]: CurrentMarkers)
		{
			const float Scale = DrawMarkerDebugSphere->_Sphere.Radius * 0.1f;
			const Matrix World = FMath::WorldMatrix({ Scale,Scale,Scale }, 
													{ 0,0,0 }, 
													{DrawMarkerDebugSphere->_Sphere.Center });
			Device->SetTransform(D3DTS_WORLD, &World);
			DebugSphereMesh->DrawSubset(0);
		}
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}



std::optional<std::pair<Vector3, const Engine::Cell*>> 
	Engine::NavigationMesh::MoveOnNavigation(
		const Vector3 TargetDirection, 
		const Vector3 TargetLocation, 
		const Cell* CurrentPlacedCell) const&
{
	const Vector3 EndLocation = TargetLocation + TargetDirection;
	const auto [_Result, NewPlacedCell] =
		CurrentPlacedCell->Compare(EndLocation);

	switch (_Result)
	{
	case Engine::Cell::CompareType::Moving:
		return { { TargetDirection,NewPlacedCell } };
		break;
	case Engine::Cell::CompareType::Stop:
		return std::nullopt;
		break;
	default:
		return std::nullopt;
		break;
	}
}
