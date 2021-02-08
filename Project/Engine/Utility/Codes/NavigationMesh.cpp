#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>

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

				// ��ĿŰ���� ��Ŀ�� �����ϴ� ������ ���� �� ���ԵǹǷ�
				// �̿��� �ش� ���� ���� �������� �̿����� ����.
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
			// ���� ����� ��Ŀ�鿡�� ������ ����.
			auto DeleteCell = *CurCell;
			iter = CellContainer.erase(iter); 
			for (const uint32 DeleteCellKey : DeleteCell.MarkerKeys)
			{
				CurrentMarkers.erase(DeleteCellKey);
			}
			continue;
		}
		else
		{
			++iter;
		}
	}
}

bool Engine::NavigationMesh::InsertPointFromMarkers(const Ray WorldRay)&
{
	for (auto& [MarkerKey, CurTargetMarker] : CurrentMarkers)
	{
		float t0, t1; Vector3 IntersectPt;
		const Sphere TargetSphere = CurTargetMarker->_Sphere;
		if (FMath::IsRayToSphere(WorldRay, TargetSphere, t0, t1, IntersectPt))
		{
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
				CurTargetMarker->SharedCellKeys.push_back(CurrentCellKey);
			}

			return true; 
		}
	}
	return false;
}

void Engine::NavigationMesh::InsertPoint(const Vector3 Point)&
{
	auto _Marker = std::make_shared< Marker>();
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
			CurrentMarkers.find(PickKey)->second->SharedCellKeys.push_back(CurrentCellKey);
		}
		CurrentPickPoints.clear();
	}
}

void Engine::NavigationMesh::Initialize(IDirect3DDevice9* Device)&
{
	this->Device = Device;

	Device->CreateVertexBuffer(sizeof(Vertex::LocationColor) * 3u*3000u, D3DUSAGE_DYNAMIC, Vertex::LocationColor::FVF,D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
	ResourceSystem::Instance->Insert<IDirect3DVertexBuffer9>(L"NaviMeshDebugVertexBuffer", VertexBuffer);
	// ���⼭ ���� �ε�...
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

				VtxBufPtr[TargetBufferIdx].Diffuse = 0x7B7D7D7D;
			}
			++Idx;
		}

		VertexBuffer->Unlock();
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