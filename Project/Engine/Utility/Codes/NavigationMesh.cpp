#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "Cell.h"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>

void Engine::NavigationMesh::Save(const std::filesystem::path SavePath) const&
{
	std::wofstream Os{ SavePath };
	for (const auto& SaveCell : CellContainer)
	{
		Os << L"Cell [" << SaveCell->Index << "]"
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

			CurrentMarkers.erase(
				std::remove_if(std::begin(CurrentMarkers), std::end(CurrentMarkers) , 
					[DeleteCell](const Sphere _Marker)
					{
						return (FMath::Equal(DeleteCell.PointA, _Marker.Center)||
								FMath::Equal(DeleteCell.PointB, _Marker.Center) ||
								FMath::Equal(DeleteCell.PointC, _Marker.Center)
							);
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

		if (FMath::IsRayToSphere(WorldRay, CurTargetMarker, t0, t1, IntersectPt))
		{
			InsertPoint(CurTargetMarker.Center,false);
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
		Sphere _Sphere;
		_Sphere.Radius = 1.f;
		_Sphere.Center = Point;
		CurrentMarkers.push_back(_Sphere);
	}

	if (CurrentPickPoints.size() >= 3u)
	{
		const uint32 CellContainerIdx = CellContainer.size();
		auto _InsertCell = std::make_shared<Cell>();
		_InsertCell->Initialize(this,CellContainerIdx,
								CurrentPickPoints[0],
								CurrentPickPoints[1],
								CurrentPickPoints[2] ,
								Device);
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
			const float Scale = DrawMarkerDebugSphere.Radius * 0.1f;
			const Matrix World = FMath::WorldMatrix({ Scale,Scale,Scale }, 
													{ 0,0,0 }, 
													{DrawMarkerDebugSphere.Center });
			Device->SetTransform(D3DTS_WORLD, &World);
			DebugSphereMesh->DrawSubset(0);
		}
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
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
