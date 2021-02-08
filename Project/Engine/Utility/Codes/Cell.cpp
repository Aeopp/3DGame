#include "Cell.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include <array>
#include "NavigationMesh.h"

void Engine::Cell::Initialize(
	Engine::NavigationMesh* const NaviMesh,
	const Vector3& PointA,
	const Vector3& PointB, 
	const Vector3& PointC,
	IDirect3DDevice9*const Device)&
{
	this->PointA = PointA;
	this->PointB = PointB;
	this->PointC = PointC;
	const Vector2 PointA_XZ{ PointA.x,PointA.z}; 
	const Vector2 PointB_XZ{ PointB.x,PointB.z };
	const Vector2 PointC_XZ{ PointC.x,PointC.z };
	Segment2DAB = Segment2DAndNormal::Make(PointA_XZ, PointB_XZ);
	Segment2DBC = Segment2DAndNormal::Make(PointB_XZ, PointC_XZ);
	Segment2DCA = Segment2DAndNormal::Make(PointC_XZ, PointA_XZ);

	static uint32 LineResourceIdx = 0u; 
	if (Engine::Global::bDebugMode)
	{
		D3DXCreateLine(Device, &Line);
		ResourceSystem::Instance->Insert<ID3DXLine>(L"Line"+std::to_wstring(LineResourceIdx++), Line);
		Line->SetWidth(2.5f);
	}
}

bool Engine::Cell::FindNeighbor(
	const Vector3& PointFirst, const Vector3& PointSecond, Cell* _Cell)&
{
	if (FMath::Equal(PointFirst, PointA))
	{
		if (FMath::Equal(PointSecond, PointB))
		{
			NeighborAB = _Cell;
			return true;
		}

		if (FMath::Equal(PointSecond, PointC))
		{
			NeighborCA = _Cell;
			return true; 
		}
	}

	if (FMath::Equal(PointFirst, PointB))
	{
		if (FMath::Equal(PointSecond, PointA))
		{
			NeighborAB = _Cell;
			return true;
		}

		if (FMath::Equal(PointSecond, PointC))
		{
			NeighborBC= _Cell;
			return true;
		}
	}

	if (FMath::Equal(PointFirst, PointC))
	{
		if (FMath::Equal(PointSecond, PointB))
		{
			NeighborBC = _Cell;
			return true;
		}

		if (FMath::Equal(PointSecond, PointA))
		{
			NeighborCA = _Cell;
			return true;
		}
	}

	return false;
}

void Engine::Cell::Render(IDirect3DDevice9* Device)&
{
	if (false == Engine::Global::bDebugMode)return;

	std::array<Vector3, 4ul >Points{};
	Points[0] = PointA;
	Points[1] = PointB;
	Points[2] = PointC;
	Points[3] = PointA;

	Matrix View;
	Matrix Projection;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);

	for (uint32 i = 0; i < 4; ++i)
	{
		D3DXVec3TransformCoord(&Points[i], &Points[i], &View);
		Points[i].z = (std::fmaxf)(0.1f, Points[i].z);
		D3DXVec3TransformCoord(&Points[i], &Points[i], &Projection);
	}
	
	Line->Begin();
	Matrix Identity = FMath::Identity();
	Line->DrawTransform(Points.data(),Points.size(),
		&Identity, D3DXCOLOR(1.f, 0.f, 0.f, 1.f));

	Line->End();

	
}

std::pair<Engine::Cell::CompareType,const Engine::Cell*>
Engine::Cell::Compare(const Vector3& EndPosition) const&
{
	const Vector2 EndPosition2D = { EndPosition.x, EndPosition.z };

	auto IsOutLine = [EndPosition2D,this](const Segment2DAndNormal& TargetSegment)
	{
		const Vector2 ToEnd = EndPosition2D - TargetSegment.Begin;
		const float d = D3DXVec2Dot(&TargetSegment.Normal, &ToEnd);
		return (d >= 0.0f); 
	};

	static auto CompareImplementation = 
		[](Engine::Cell* TargetNeighbor)
				-> std::pair<Engine::Cell::CompareType, const Engine::Cell*>
	{
			if (nullptr == TargetNeighbor)
				return  { Cell::CompareType::Stop  , 0u };
			else 
				return  { Cell::CompareType::Moving ,TargetNeighbor };
	};
	// 방향과 노말 내적 해서 부호가 예각이면 바깥 라인 
	// 둔각이면 아래 라인
	for (uint32 i = 0; i < 4; ++i)
	{
		if (IsOutLine(Segment2DAB))
		{
			return CompareImplementation(NeighborAB);
		}
		else if (IsOutLine(Segment2DBC))
		{
			return CompareImplementation(NeighborBC);
		}
		else if (IsOutLine(Segment2DCA))
		{
			return CompareImplementation(NeighborCA);
		}
	}

	return { Engine::Cell::CompareType::Moving,this};
}


