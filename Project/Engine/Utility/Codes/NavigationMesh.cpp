#include "NavigationMesh.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include <algorithm>
#include "Vertexs.hpp"
#include <ostream>
#include <fstream>
#include <set>
#include <sstream>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

static uint32 MarkerKey{ 1u };
static uint32 CellKey{ 1u };

void Engine::NavigationMesh::Clear()
{
	CellContainer.clear();
	CurrentMarkers.clear();
}

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
void Engine::NavigationMesh::Save(const std::filesystem::path SavePath,const Matrix& MapWorld) &
{
	const Matrix ToMapLocal = FMath::Inverse(MapWorld);

	using namespace rapidjson;

	StringBuffer StrBuf;

	PrettyWriter<StringBuffer> Writer(StrBuf);

	// Cell Information Write...
	Writer.StartObject();
	Writer.Key("CellList");
	Writer.StartArray();
	for (const auto& [CellKey, _Cell] : CellContainer)
	{
		Writer.StartObject();
		{
			{
				{
					Writer.Key("Index");
					Writer.Uint(CellKey);
				}
			}
			{
				Writer.Key("Point A");
				Writer.StartArray();
				{
					const Vector3 LocalPointA = FMath::Mul(_Cell->PointA, ToMapLocal);
					Writer.Double(LocalPointA.x);
					Writer.Double(LocalPointA.y);
					Writer.Double(LocalPointA.z);
				}
				Writer.EndArray();
			}
			{
				Writer.Key("Point B");
				Writer.StartArray();
				{
					const Vector3 LocalPointB = FMath::Mul(_Cell->PointB, ToMapLocal);
					Writer.Double(LocalPointB.x);
					Writer.Double(LocalPointB.y);
					Writer.Double(LocalPointB.z);
				}
				Writer.EndArray();
			}
			{
				Writer.Key("Point C");
				Writer.StartArray();
				{
					const Vector3 LocalPointC = FMath::Mul(_Cell->PointC, ToMapLocal);
					Writer.Double(LocalPointC.x);
					Writer.Double(LocalPointC.y);
					Writer.Double(LocalPointC.z);
				}
				Writer.EndArray();
			}
			{
				Writer.Key("bEnableJumping");
				Writer.Bool(_Cell->bEnableJumping);
			}
			Writer.Key("NeighborList");
			Writer.StartArray();
			{
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
					Writer.Uint(NeighborKey);
				}
			}
			Writer.EndArray();
			
			Writer.Key("MarkerSet");
			Writer.StartArray();
			{
				for (const uint32 MarkerKey : _Cell->MarkerKeys)
				{
					Writer.Uint(MarkerKey);
				}
			}
			Writer.EndArray();
		}
		Writer.EndObject();
	}
	Writer.EndArray();

	// Marker Parser..
	Writer.Key("MarkerList");
	{
		Writer.StartArray();
		for (const auto& [MarkerKey, _Marker] : CurrentMarkers)
		{
			Writer.StartObject();
			{
				{
					Writer.Key("Index");
					Writer.Uint(MarkerKey);
				}
				{
					Writer.Key("Location");
					Writer.StartArray();
					const Vector3 MarkerWorldLocation = _Marker->_Sphere.Center;
					const Vector3 MarkerLocalLocation = FMath::Mul(MarkerWorldLocation, ToMapLocal); 
					Writer.Double(MarkerLocalLocation.x);
					Writer.Double(MarkerLocalLocation.y);
					Writer.Double(MarkerLocalLocation.z);
					Writer.EndArray();
				}
				{
					Writer.Key("SharedCellIndexList");
					Writer.StartArray();
					for (const uint32 SharedCellKey : _Marker->SharedCellKeys)
					{
						Writer.Uint(SharedCellKey);
					}
					Writer.EndArray();
				}
			}
			Writer.EndObject();
		}
		Writer.EndArray();
	}

	Writer.EndObject();
	std::ofstream Of{ SavePath };
	NaviMeshInfoString = StrBuf.GetString();
	Of << NaviMeshInfoString;
}

void Engine::NavigationMesh::Load(const std::filesystem::path LoadPath,const Matrix& MapWorld)&
{
	std::ifstream Is{ LoadPath };
	using namespace rapidjson;
	if (!Is.is_open()) return;

	IStreamWrapper Isw(Is);
	Document _Document;
	_Document.ParseStream(Isw);

	if (_Document.HasParseError())
	{
		MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
		return;
	}

	CellContainer.clear();
	CurrentMarkers.clear();

	const Value& CellList = _Document["CellList"];
	for (auto CellIterator = CellList.Begin();
		CellIterator != CellList.End(); ++CellIterator)
	{
		std::shared_ptr<Cell> PushCell = std::make_shared<Cell>();

		const uint32 CellIndex = CellIterator->FindMember("Index")->value.GetUint();
		const auto PointAxyz   = CellIterator->FindMember ("Point A")->value.GetArray();
		const auto PointBxyz =   CellIterator->FindMember("Point B")->value.GetArray();
		const auto PointCxyz =   CellIterator->FindMember("Point C")->value.GetArray();

		const Vector3 LocalPointA  { PointAxyz[0].GetFloat(),PointAxyz[1].GetFloat(), PointAxyz[2].GetFloat() };
		const Vector3 LocalPointB  { PointBxyz[0].GetFloat(),PointBxyz[1].GetFloat(), PointBxyz[2].GetFloat() };
		const Vector3 LocalPointC  { PointCxyz[0].GetFloat(),PointCxyz[1].GetFloat(), PointCxyz[2].GetFloat() };

		const Vector3 WorldPointA =FMath::Mul(LocalPointA,MapWorld); 
		const Vector3 WorldPointB =FMath::Mul(LocalPointB,MapWorld); 
		const Vector3 WorldPointC =FMath::Mul(LocalPointC,MapWorld); 

		bool bEnableJumping = false;

		if (CellIterator->HasMember("bEnableJumping"))
		{
			bEnableJumping =CellIterator->FindMember("bEnableJumping")->value.GetBool();
		}


		std::array<uint32,3u> MarkerKeySet{};
		const auto MarkerSet = CellIterator->FindMember("MarkerSet")->value.GetArray();
		for (uint32 i = 0; i < MarkerKeySet.size(); ++i)
		{
			MarkerKeySet[i] = MarkerSet[i].GetUint();
		}
		PushCell->Initialize(this, WorldPointA, WorldPointB, WorldPointC, Device,MarkerKeySet , bEnableJumping);
		CellContainer.insert({ CellIndex ,std::move(PushCell) });
	}
	const Value& MarkerList = _Document["MarkerList"];
	for (auto MarkerIter = MarkerList.Begin();
		MarkerIter != MarkerList.End(); ++MarkerIter)
	{
		std::shared_ptr<Marker> PushMarker = std::make_shared<Marker >();
		const uint32 MarkerIndex = MarkerIter->FindMember("Index")->value.GetUint();
		const auto MarkerLocalArray = MarkerIter->FindMember("Location")->value.GetArray();

		const Vector3 MarkerLocalLocation{
			MarkerLocalArray[0].GetFloat()  ,
			MarkerLocalArray[1].GetFloat()  ,
			MarkerLocalArray[2].GetFloat()}; 

		const Vector3 MarkerWorldLocation{ FMath::Mul(MarkerLocalLocation,MapWorld) }; 

		PushMarker->_Sphere.Center = MarkerWorldLocation;

		const auto SharedCellIndexArray = MarkerIter->FindMember("SharedCellIndexList")->value.GetArray();
		for (auto SharedCellIter = SharedCellIndexArray.begin();
			SharedCellIter != SharedCellIndexArray.end(); ++SharedCellIter)
		{
			PushMarker->SharedCellKeys.insert(SharedCellIter->GetUint());
		};
		PushMarker->_Sphere.Radius = 1.f;
		CurrentMarkers.insert({ MarkerIndex , std::move (PushMarker)  });
	}

	if (auto MaxIter = std::max_element(
		std::begin(CurrentMarkers),
		std::end(CurrentMarkers), [](const auto& Lhs, const auto& Rhs)
		{
			return Lhs.first < Rhs.first;
		}); MaxIter != std::end(CurrentMarkers))
	{
		MarkerKey = (MaxIter->first + 1u);
	}

	if (auto MaxIter = std::max_element(
		std::begin(CellContainer),
		std::end(CellContainer), [](const auto& Lhs, const auto& Rhs)
		{
			return Lhs.first < Rhs.first;
		}); MaxIter != std::end(CellContainer))
	{
		CellKey = (MaxIter->first + 1u);
	}


	std::stringstream StringStream;
	StringStream<<Is.rdbuf();
	NaviMeshInfoString = StringStream.str();
	CellNeighborLink();

	for (const auto& [Key,_Cell] : CellContainer)
	{
		if (_Cell->bEnableJumping)
		{
			CellContainerEnableJumping.push_back(_Cell.get());
		}
	}
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
		auto SelectCell = iter->second;

		ImGui::Separator();
		ImGui::Text("Select Cell Edit"); 
		ImGui::Text("Cell Index : %d ", CurSelectCellKey);
		ImGui::SameLine();
		ImGui::Checkbox("bEnableJumping", &SelectCell->bEnableJumping);
		ImGui::Text("Marker Index..."); 	ImGui::SameLine();
	
		
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
	if (false == NaviMeshInfoString.empty())
	{
		if (ImGui::CollapsingHeader("Data content Json Format"))
		{
			ImGui::Text(NaviMeshInfoString.c_str());
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

uint32 Engine::NavigationMesh::InsertPointFromMarkers(const Ray WorldRay ,
	const bool bEnableJumping)&
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
						CurrentPickPoints[2].first },
					bEnableJumping
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

uint32 Engine::NavigationMesh::InsertPoint(const Vector3 Point , const bool bEnableJumping)&
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
				CurrentPickPoints[2].first },
			bEnableJumping
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

	ForwardShaderFx=Engine::ShaderFx::Load(Device, Engine::Global::ResourcePath / L"Shader" / L"NaviMeshFx.hlsl", L"NaviMeshFx");
	VtxDecl = Engine::ResourceSystem::Instance->Insert <IDirect3DVertexDeclaration9>(L"VertexDecl_LocationColor",
		Vertex::LocationColor::GetVertexDecl(Device));

	Device->CreateVertexBuffer(sizeof(Vertex::LocationColor) * 3u*3000u, D3DUSAGE_DYNAMIC, Vertex::LocationColor::FVF,D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
	ResourceSystem::Instance->Insert<IDirect3DVertexBuffer9>(L"NaviMeshDebugVertexBuffer", VertexBuffer);
	// 여기서 파일 로딩...
}

void Engine::NavigationMesh::Render(IDirect3DDevice9* const Device)&
{
	if (Engine::Global::bDebugMode)
	{
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
			sizeof(Vertex::LocationColor) * 3u * CellContainer.size(), reinterpret_cast<void**>(&VtxBufPtr), NULL);

		uint32 Idx = 0u;
		for (const auto& [CellKey, _Cell] : CellContainer)
		{
			static const Vector4 EnableJumpingColor = {0.55f,0.12f,0.24f,0.15f };
			Vector4 Diffuse = FilterAfterNeighborKeys.contains(CellKey) ?
				NeighborColor : DefaultColor;

			if (CellKey == CurSelectCellKey)
			{
				Diffuse = SelectColor;
			}

			if (_Cell->bEnableJumping)
			{
				Diffuse += EnableJumpingColor;
			}

			for (uint32 i = 0; i < 3; ++i)
			{
				const uint32 TargetBufferIdx = Idx * 3u + i;
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

		Matrix Projection, View;
		Device->GetTransform(D3DTS_PROJECTION, &Projection);
		Device->GetTransform(D3DTS_VIEW, &View);
		ID3DXEffect* Fx = ForwardShaderFx->GetHandle();
		Fx->SetMatrix("World", &World);
		Fx->SetMatrix("View", &View);
		Fx->SetMatrix("Projection", &Projection);
		uint32 PassNum = 0;
		Fx->Begin(&PassNum, 0);
		for (uint32 PassIdx = 0u; PassIdx < PassNum; ++PassIdx)
		{
			Fx->BeginPass(PassIdx);
			Device->SetVertexDeclaration(VtxDecl);
			/*Device->SetFVF(Vertex::LocationColor::FVF);*/
			Device->SetStreamSource(0, VertexBuffer, 0u, sizeof(Vertex::LocationColor));
			Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0u, CellContainer.size());
			Fx->EndPass();
		};
		Fx->End();

		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		auto DebugSphereMesh = ResourceSystem::Instance->Get<ID3DXMesh>(L"SphereMesh");
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		for (auto& [MarkerKey, DrawMarkerDebugSphere] : CurrentMarkers)
		{
			const float Scale = DrawMarkerDebugSphere->_Sphere.Radius;
			const Matrix World = FMath::WorldMatrix({ Scale,Scale,Scale },
				{ 0,0,0 },
				{ DrawMarkerDebugSphere->_Sphere.Center });
			Device->SetTransform(D3DTS_WORLD, &World);
			DebugSphereMesh->DrawSubset(0);
		}
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		DWORD bZEnable{ 0u };
		Device->GetRenderState(D3DRS_ZENABLE, &bZEnable);
		Device->SetRenderState(D3DRS_ZENABLE, true);
		for (auto& [CellKey, CurCell] : CellContainer)
		{
			CurCell->Render(Device);
		}
		Device->SetRenderState(D3DRS_ZENABLE, bZEnable);
	}
}


Engine::Cell* Engine::NavigationMesh::GetCellFromXZLocation(const Vector2& Position2D) const&
{
	for (const auto& [CellKey, _Cell] : CellContainer)
	{
		if (_Cell->IsOutLine(Position2D).has_value()==false)
		{
			return _Cell.get();
		}
	}

	return nullptr;
}

Engine::Cell* Engine::NavigationMesh::GetJumpingCellFromXZLocation(const Vector2& Position2D) const&
{
	for (const auto& _Cell : CellContainerEnableJumping)
	{
		if (_Cell->IsOutLine(Position2D).has_value() == false)
		{
			return _Cell;
		}
	}

	return nullptr;

}

std::vector<Engine::Cell*> Engine::NavigationMesh::GetCellFromMarkerIdx(const uint32 MarkerIdx) const&
{
	std::vector<Cell*> CellList{};

	const auto& TargetMarker = CurrentMarkers.find(MarkerIdx)->second;
	for (const uint32 SharedCellKey : TargetMarker->SharedCellKeys)
	{
		const auto& TargetCell = CellContainer.find(SharedCellKey)->second;
		CellList.push_back(TargetCell.get( ) );
	}

	return CellList; 
}
