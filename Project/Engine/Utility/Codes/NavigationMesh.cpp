#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>
#include <set>
#include <sstream>
#include "json/json.h"


static uint32 MarkerKey{ 1u };
static uint32 CellKey{ 1u };

void Engine::NavigationMesh::CellNeighborLink()&
{
	for (auto& [CellKey,_Cell] : CellContainer)
	{
		_Cell->Neighbors.clear();
		// 필터링 작업. 마커를 여러개를 공유해서 하나의 인스턴스가 중복으로 이웃으로 삽입되는 것과
		std::set<uint32> NeighborKeyset{};
		for (const uint32 CurCellMarkerKey : _Cell->MarkerKeys)
		{
			for (const uint32 NeighborTargetCellKey :
				CurrentMarkers.find(CurCellMarkerKey)->second->SharedCellKeys)
			{
				NeighborKeyset.insert(NeighborTargetCellKey);
			}
		}
		// 자기자신이 이웃이 되는 경우를 방지.
		NeighborKeyset.erase(CellKey);
		for (const uint32 NeighborKey : NeighborKeyset)
		{
			Cell* NeighborTarget = CellContainer.find(NeighborKey)->second.get();
			if (NeighborTarget != _Cell.get())
			{
				_Cell->Neighbors.push_back(NeighborTarget);
			}
		}
	}
}

void Engine::NavigationMesh::SaveFile(const std::filesystem::path SavePath)&
{
	std::ofstream Os{ SavePath };
	std::stringstream Wss;

	for (const auto& [CellKey, SaveCell] : CellContainer)
	{
		Wss << "Cell [ " << CellKey << " ]" << std::endl
			<< "Location A [ " << SaveCell->PointA.x << " " << SaveCell->PointA.y << " " << SaveCell->PointA.z << " ]"
			<< "  B [ " << SaveCell->PointB.x << " " << SaveCell->PointB.y << " " << SaveCell->PointB.z << " ]"
			<< "  C [ " << SaveCell->PointC.x << " " << SaveCell->PointC.y << " " << SaveCell->PointC.z << " ]" << std::endl
			<< "Neighbor [ ";

		std::set<uint32> NeighborKeyset;
		for (const uint32 SaveCellMarkerKey : SaveCell->MarkerKeys)
		{
			for (const auto& [MarkerKey, _Marker] : CurrentMarkers)
			{
				for (const auto& _MaybeNeighborKey : _Marker->SharedCellKeys)
				{
					NeighborKeyset.insert(_MaybeNeighborKey);
				}
			}
		}
		NeighborKeyset.erase(CellKey);
		for (const uint32 NeighborKey : NeighborKeyset)
		{
			Wss << NeighborKey << " ";
		}
		Wss << " ]\n" << std::endl;
	}
	Wss << "---------------------------------------------------------------------------------------------------\n";
	for (const auto& [MarkerKey, _Marker] : CurrentMarkers)
	{
		Wss << "Marker [ " << MarkerKey << " ]\n";
		Wss << "CellKey [ ";
		for (const uint32 SharedCellKey : _Marker->SharedCellKeys)
		{
			Wss << SharedCellKey << " ";
		}
		Wss << "]\n\n";
	}
	Os << Wss.str();

}

void Engine::NavigationMesh::Save(const std::filesystem::path SavePath) &
{
	std::ofstream Os{ SavePath };
	std::stringstream Wss{}; 
	Json::Value Root{}; 
	Json::StreamWriterBuilder Builder{};
	const std::unique_ptr<Json::StreamWriter> Writer(Builder.newStreamWriter());

	for (const auto& [CellKey, _Cell] : CellContainer)
	{
		Json::Value Location, CellInfo, NeighborList;

		Location["A"].append(_Cell->PointA.x);
		Location["A"].append(_Cell->PointA.y);
		Location["A"].append(_Cell->PointA.z);

		Location["B"].append(_Cell->PointB.x);
		Location["B"].append(_Cell->PointB.y);
		Location["B"].append(_Cell->PointB.z);

		Location["C"].append(_Cell->PointC.x);
		Location["C"].append(_Cell->PointC.y);
		Location["C"].append(_Cell->PointC.z);

	

		std::set<uint32> NeighborKeyset;
		for (const uint32 SaveCellMarkerKey : _Cell->MarkerKeys)
		{
			for (const auto& [MarkerKey, _Marker] : CurrentMarkers)
			{
				for (const auto& _MaybeNeighborKey : _Marker->SharedCellKeys)
				{
					NeighborKeyset.insert(_MaybeNeighborKey);
				}
			}
		}
		NeighborKeyset.erase(CellKey);
		for (const uint32 NeighborKey : NeighborKeyset)
		{
			NeighborList.append(NeighborKey);
		}
		CellInfo["NeighborList"] = NeighborList;
		CellInfo["Key"] = CellKey;
		CellInfo["Location"] = Location;

		Root["Cell"].append(CellInfo);
	}

	for (const auto& [MarkerKey, _Marker] : CurrentMarkers)
	{
		Json::Value MarkerInfo{};

		MarkerInfo["Key"] = MarkerKey;

		for (const uint32 SharedCellKey : _Marker->SharedCellKeys)
		{
			MarkerInfo["CellKeyList"].append(SharedCellKey);
		}

		Root["Marker"].append(MarkerInfo);
	}

	Writer->write(Root, &Os);
	Writer->write(Root, &LastSaveFileBuffer);
}

void Engine::NavigationMesh::Load(const std::filesystem::path LoadPath)&
{
	std::ifstream Is{ LoadPath };
	CellContainer.clear();
	CurrentMarkers.clear();
	constexpr std::streamsize BufferSize = 256u;
	char Buffer[BufferSize];
	Is.getline(Buffer, BufferSize,'[');
	uint32 CellIndex;
	Is >> CellIndex;
	
	CellContainer.insert({});
}

void Engine::NavigationMesh::DebugLog()&
{
	ImGui::Begin("NavigationMesh Information");
	ImGui::Separator();
	ImGui::Text("Cell Count : %d | Marker Count : %d", CellContainer.size(), CurrentMarkers.size());
	ImGui::Separator();
	if (auto iter = CellContainer.find(CurSelectCellKey);
		iter != std::end( CellContainer ))
	{
		ImGui::Separator();
		ImGui::Text("Select Cell Information"); 
		ImGui::Text("Cell Index : %d ", CurSelectCellKey);
		ImGui::Text("Marker Index..."); 	ImGui::SameLine();
	
		auto SelectCell = iter->second;
		for (const uint32 MarkerKey : SelectCell->MarkerKeys)
		{
			ImGui::SameLine();
			ImGui::Text("%d ", MarkerKey);
		}
		ImGui::Text("Neighbor Keys... :");
		ImGui::SameLine();
		std::set<uint32> FilterAfterNeighborKeys{};
		for (const uint32 MarkerKey : SelectCell->MarkerKeys)
		{
			if (auto MarkerIter = CurrentMarkers.find(MarkerKey);
				MarkerIter != std::end(CurrentMarkers))
			{
				for (const uint32 NeighborKey : MarkerIter->second->SharedCellKeys)
				{
					FilterAfterNeighborKeys.insert(NeighborKey);
				}
			}
		}
		FilterAfterNeighborKeys.erase(CurSelectCellKey);
		for (const uint32 FilterAfterNeighborKey : FilterAfterNeighborKeys)
		{
			ImGui::SameLine();
			ImGui::Text("%d ", FilterAfterNeighborKey);
		}
	}

	ImGui::Separator();
	if (auto iter = CurrentMarkers.find(CurSelectMarkerKey);
		iter != std::end(CurrentMarkers))
	{
		auto SelectMarker = iter->second;
		ImGui::Text("Select Marker Information");
		ImGui::Text("Marker Index : %d ", CurSelectMarkerKey);
		ImGui::Text("Marker Point Shared Cell"); 
		for (const uint32 MarkerPointSharedCell : SelectMarker->SharedCellKeys)
		{
			ImGui::SameLine();
			ImGui::Text("%d ", MarkerPointSharedCell);
			
		}
		ImGui::Separator();
	}
	if (false == LastSaveFileBuffer.str().empty())
	{
		if (ImGui::CollapsingHeader("Saved data content"))
		{
			ImGui::Text(LastSaveFileBuffer.str().c_str());
		}
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
			for (const uint32 DeleteMarkerKey : DeleteCell->MarkerKeys)
			{
				auto MarkerIter = CurrentMarkers.find(DeleteMarkerKey);
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
				const uint32 CurrentCellKey = CellKey++;  
				CellContainer.insert({ CurrentCellKey ,std::move(_InsertCell) });
				// 셀을 푸시하면서 셀에 해당하는 마커에 셀 인덱스 푸시.
				for (uint32 i = 0; i < CurrentPickPoints.size();++i)
				{
					CurrentMarkers.find(CurrentPickPoints[i].first)->second->SharedCellKeys.insert(CurrentCellKey);
				}
				CurrentPickPoints.clear();
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
	const uint32 CurrentMarkerKey = MarkerKey++;
	CurrentMarkers.insert({ CurrentMarkerKey,_Marker});
	CurrentPickPoints.push_back({ CurrentMarkerKey, Point });

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

	return CurrentMarkerKey;
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

		std::set<uint32> FilterAfterNeighborKeys{};

		if (auto iter = CellContainer.find(CurSelectCellKey);
			iter != std::end(CellContainer))
		{
			auto SelectCell = iter->second;
			for (const uint32 MarkerKey : SelectCell->MarkerKeys)
			{
				if (auto MarkerIter = CurrentMarkers.find(MarkerKey);
					MarkerIter != std::end(CurrentMarkers))
				{
					for (const uint32 NeighborKey : MarkerIter->second->SharedCellKeys)
					{
						FilterAfterNeighborKeys.insert(NeighborKey);
					}
				}
			}
			FilterAfterNeighborKeys.erase(CurSelectCellKey);
		}

		Vertex::LocationColor* VtxBufPtr{ nullptr };
		VertexBuffer->Lock(0u,
		sizeof(Vertex::LocationColor) * 3u * CellContainer.size(), reinterpret_cast<void**>			(&VtxBufPtr),NULL);
		
		uint32 Idx = 0u;
		for (const auto& [CellKey, _Cell] : CellContainer)
		{
			 D3DCOLOR Diffuse= FilterAfterNeighborKeys.contains(CellKey) ? 
				 D3DCOLOR_COLORVALUE(1.f,0.f,0.f,1.f) : D3DCOLOR_COLORVALUE(0.f,1.f,0.f,1.f) ;
			 Diffuse =  ( (CurSelectCellKey!=0u) &&(Idx== CurSelectCellKey) )? 
				 D3DCOLOR_COLORVALUE(0.f, 0.f, 1.f, 1.f) : Diffuse;
			for (uint32 i = 0; i < 3; ++i)
			{
				const uint32 TargetBufferIdx =Idx * 3u + i;
				if (i == 0)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointA;
				if (i == 1)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointB;
				if (i == 2)
					VtxBufPtr[TargetBufferIdx].Location = _Cell->PointC;

				VtxBufPtr[TargetBufferIdx].Diffuse = Diffuse;
			}
			++Idx;
		}
		VertexBuffer->Unlock();
		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
		//Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
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
		for (auto& [MarkerKey,DrawMarkerDebugSphere]: CurrentMarkers)
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
