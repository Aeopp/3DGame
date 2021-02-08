#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>

void Engine::NavigationMesh::CellNeighborLink()&
{

}

void Engine::NavigationMesh::Save(const std::filesystem::path SavePath) const&
{
	std::wofstream Os{ SavePath };
	for (const auto& SaveCell : CellContainer)
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
	size_t Idx = 0u;
	for (auto iter = std::begin(CellContainer); 
			iter!= std::end(CellContainer) ;)
	{
		auto CurCell = *iter;
		const PlaneInfo CurPlane = 
			PlaneInfo::Make({ CurCell->PointA  ,CurCell->PointB, CurCell->PointC});
		float t; 
		Vector3 IntersectPoint;
		if (FMath::IsTriangleToRay(CurPlane, WorldRay, t, IntersectPoint))
		{
			const Cell DeleteCell = *(*iter); 
			iter = CellContainer.erase(iter); 

			// 지운 셀의 포인트와 중심을 공유하던 마커를 탐색하며
			// 마커의 중심이 다른 남아있는 셀과 공유중이라면 지우지 않고 그렇지 않다면 지운다.

			CurrentMarkers.erase(
				std::remove_if(std::begin(CurrentMarkers), std::end(CurrentMarkers) , 
					[DeleteCell,this](const Sphere _Marker)
					{
						if   ( FMath::Equal(DeleteCell.PointA, _Marker.Center) ||
								FMath::Equal(DeleteCell.PointB, _Marker.Center) ||
								FMath::Equal(DeleteCell.PointC, _Marker.Center))
						{
							for (const auto& CheckRemainCell : CellContainer)
							{
								if (FMath::Equal(CheckRemainCell->PointA, _Marker.Center) ||
									FMath::Equal(CheckRemainCell->PointA, _Marker.Center) || 
									FMath::Equal(CheckRemainCell->PointA, _Marker.Center))
									return false;
							}

							return true;
						}
						return false;
					}),
					std::end(CurrentMarkers) );

			continue;
		}
		else
		{
			++Idx;
			++iter;
		}
	}
}

bool Engine::NavigationMesh::InsertPointFromMarkers(const Ray WorldRay)&
{
	for (auto& CurTargetMarker : CurrentMarkers)
	{
		float t0, t1; Vector3 IntersectPt; 
		const Sphere TargetSphere = CurTargetMarker->_Sphere; 
		if (FMath::IsRayToSphere(WorldRay, TargetSphere, t0, t1, IntersectPt))
		{
			InsertPoint(TargetSphere.Center,false);
			return true; 
		}
	}

	return false;
}

void Engine::NavigationMesh::InsertPoint(const Vector3 Point,
										 const bool bCreateMarker)&
{
	CurrentPickPoints.push_back(Point);

	if (bCreateMarker)
	{
		auto _Marker = CurrentMarkers.emplace_back(std::make_shared<Marker>());
		_Marker->_Sphere.Radius = 1.f;
		_Marker->_Sphere.Center = Point;
	}

	if (CurrentPickPoints.size() >= 3u)
	{
		const uint32 CellContainerIdx = CellContainer.size();
		auto _InsertCell = std::make_shared<Cell>();
		_InsertCell->Initialize(this,
					CurrentPickPoints[0],
					CurrentPickPoints[1],
					CurrentPickPoints[2],
					Device);
		for (auto& CurrenrMarker : CurrentMarkers)
		{
			const Vector3   MarkerPoint = CurrenrMarker->_Sphere.Center;

			for (uint32 i = 0; i < CurrentPickPoints.size(); ++i)
			{
				if (FMath::Equal(MarkerPoint,CurrentPickPoints[i]))
				{
					CurrenrMarker->SharedCells.push_back(_InsertCell.get());
				}
			}
		}
		CellContainer.push_back(std::move(_InsertCell));
		CurrentPickPoints.clear();
	}
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
		for (auto& CurCell : CellContainer)
		{
			CurCell->Render(Device);
		}
		Vertex::LocationColor* VtxBufPtr{ nullptr };
		VertexBuffer->Lock(0u,
		sizeof(Vertex::LocationColor) * 3u * CellContainer.size(), reinterpret_cast<void**>			(&VtxBufPtr),NULL);
		
		for (uint32 i = 0; i < CellContainer.size(); ++i)
		{
			for (uint32 j = 0; j < 3u; ++j)
			{
				const uint32 TargetBufferIdx = i * 3u + j; 
				if (j == 0)
					VtxBufPtr[TargetBufferIdx].Location = CellContainer[i]->PointA;
				if (j == 1)
					VtxBufPtr[TargetBufferIdx].Location = CellContainer[i]->PointB;
				if (j == 2)
					VtxBufPtr[TargetBufferIdx].Location = CellContainer[i]->PointC;
				
				VtxBufPtr[TargetBufferIdx].Diffuse = 0x7B7D7D7D;
			}
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
		for (auto& DrawMarkerDebugSphere : CurrentMarkers)
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