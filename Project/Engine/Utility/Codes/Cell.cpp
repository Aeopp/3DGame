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
	IDirect3DDevice9*const Device,
	const std::array<uint32,3u>& MarkerKeys,
	const bool bEnableJumping)&
{
	this->NaviMesh = NaviMesh;
	this->MarkerKeys = MarkerKeys; 
	this->PointA = PointA;
	this->PointB = PointB;
	this->PointC = PointC;
	this->bEnableJumping = bEnableJumping;

	_Plane = PlaneInfo::Make({ PointA, PointB, PointC });
	const Vector2 PointA_XZ  { PointA.x,PointA.z}; 
	const Vector2 PointB_XZ  { PointB.x,PointB.z };
	const Vector2 PointC_XZ  { PointC.x,PointC.z };

	Segment2DAB = Segment2DAndNormal::Make(PointA_XZ, PointB_XZ);
	Segment2DBC = Segment2DAndNormal::Make(PointB_XZ, PointC_XZ);
	Segment2DCA = Segment2DAndNormal::Make(PointC_XZ, PointA_XZ);

	static uint32 LineResourceIdx = 0u; 
	D3DXCreateLine(Device, &Line);
	ResourceSystem::Instance->Insert<ID3DXLine>(L"Line"+std::to_wstring(LineResourceIdx++), Line);
	Line->SetWidth(5.f);
}

bool Engine::Cell::FindNeighbor(
	const Vector3& PointFirst, const Vector3& PointSecond, Cell* _Cell)&
{
	//if (FMath::Equal(PointFirst, PointA))
	//{
	//	if (FMath::Equal(PointSecond, PointB))
	//	{
	//		NeighborAB = _Cell;
	//		return true;
	//	}

	//	if (FMath::Equal(PointSecond, PointC))
	//	{
	//		NeighborCA = _Cell;
	//		return true; 
	//	}
	//}

	//if (FMath::Equal(PointFirst, PointB))
	//{
	//	if (FMath::Equal(PointSecond, PointA))
	//	{
	//		NeighborAB = _Cell;
	//		return true;
	//	}

	//	if (FMath::Equal(PointSecond, PointC))
	//	{
	//		NeighborBC= _Cell;
	//		return true;
	//	}
	//}

	//if (FMath::Equal(PointFirst, PointC))
	//{
	//	if (FMath::Equal(PointSecond, PointB))
	//	{
	//		NeighborBC = _Cell;
	//		return true;
	//	}

	//	if (FMath::Equal(PointSecond, PointA))
	//	{
	//		NeighborCA = _Cell;
	//		return true;
	//	}
	//}

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

std::optional<Engine::Cell::Result>
	Engine::Cell::Compare(const Vector3& EndPosition) const&
{
	const Vector2 EndPosition2D{ EndPosition .x, EndPosition .z};
	auto bOutLine = IsOutLine(EndPosition2D);

	if ( bOutLine.has_value()==false)
	{
		const Vector3 ProjectLocation = FMath::ProjectionPointFromFace(_Plane._Plane, EndPosition);
		return    { {this,Engine::Cell::CompareType::Moving , ProjectLocation } };
	}
	else
	{
		// 해당 선분 바깥으로 나간 경우이므로 
		// 해당 선분과 이웃인 셀중 이웃이 존재하는지 검사한 이후에
		// 이웃이 없다면 이동 불가. 이웃이 존재한다면 이웃으로 셀 교체 한 이후에 이동. 
		for (const auto& NeighborCell : Neighbors)
		{
			if (auto bNeighborOutLine = NeighborCell->IsOutLine(EndPosition2D) ;
					bNeighborOutLine.has_value()==false)
			{
				const Vector3 ProjectLocation = 
					FMath::ProjectionPointFromFace(NeighborCell->_Plane._Plane, EndPosition);
				return {  {NeighborCell,Engine::Cell::CompareType::Moving,ProjectLocation } };
			};
		}

		// 이웃들 중에서도 검색에서 실패하였다.
		Vector2 ToTargetLocation = EndPosition2D - bOutLine->Begin;
		Vector2 Begin = bOutLine->Begin;
		Vector2 ToEnd = bOutLine->ToEnd;
		Vector2 End   = bOutLine->End;

		const bool bDirectionReverse = D3DXVec2Dot(&ToTargetLocation, &bOutLine->Begin) <= 0.0f;

		if (bDirectionReverse)
		{
			std::swap(Begin, End);
			ToEnd *= -1.f;
		}

		const Vector2 Force = EndPosition2D - Begin;
		Vector2 ToEndNormal;
		D3DXVec2Normalize(&ToEndNormal, &ToEnd);

		// 오브젝트가 벗어날려는 선분의 슬라이딩 벡터를 3D로 변환.
		const Vector2 OutlineSegmentSlidingVector2D = ToEndNormal * D3DXVec2Dot(&ToEndNormal, &Force);
		const Vector3 OutlineSegmentSlidingVector { OutlineSegmentSlidingVector2D.x,0.f,OutlineSegmentSlidingVector2D.y};
		// (벗어날려는 선분 시작점 + 슬라이딩 벡터) = 미끄러진 이후의 위치 
		Vector3 LocationAfterSliding { OutlineSegmentSlidingVector + Vector3{ Begin.x,0.f,Begin.y } };

		const Vector3 ToEnd3D { ToEnd.x ,0.f,ToEnd.y };
		const bool bAfterSlidingOut= FMath::Length(OutlineSegmentSlidingVector) > FMath::Length(ToEnd3D);
		if (bAfterSlidingOut)
		{
			// 슬라이딩 시킨 위치가 이웃 셀 중 어느 하나라도 내부에 존재할까 ?? 
			for (const auto& NeighborCell : Neighbors)
			{
				if (auto bNeighborOutLine = NeighborCell->IsOutLine(Vector2{ LocationAfterSliding.x,LocationAfterSliding.z });
					bNeighborOutLine.has_value() == false)
				{
					LocationAfterSliding.y = EndPosition.y;
					//해당 이웃의 내부로 이동해도 유효하니 이웃셀에 미끄러진 이후 위치 투영하고 그 위치를 반환.
					const Vector3 ProjectLocation
						{ FMath::ProjectionPointFromFace(NeighborCell->_Plane._Plane, LocationAfterSliding) };
					return { {NeighborCell,Engine::Cell::CompareType::Stop,ProjectLocation } };
				};
			}
		}

		if (bAfterSlidingOut)
		{
			LocationAfterSliding.x = End.x;
			LocationAfterSliding.z = End.y;
		}

		LocationAfterSliding.y = EndPosition.y;
		LocationAfterSliding = FMath::ProjectionPointFromFace(_Plane._Plane, LocationAfterSliding);
		return  { { this,Engine::Cell::CompareType::Stop, LocationAfterSliding} };
	}

	return std::nullopt;
};
void Engine::Cell::ReCalculateSegment2D()&
{
	const Vector2 PointA_XZ{ PointA.x,PointA.z };
	const Vector2 PointB_XZ{ PointB.x,PointB.z };
	const Vector2 PointC_XZ{ PointC.x,PointC.z };
	Segment2DAB = Segment2DAndNormal::Make(PointA_XZ, PointB_XZ);
	Segment2DBC = Segment2DAndNormal::Make(PointB_XZ, PointC_XZ);
	Segment2DCA = Segment2DAndNormal::Make(PointC_XZ, PointA_XZ);
	_Plane = PlaneInfo::Make({ PointA, PointB, PointC } );
}

 std::optional<Engine::Cell::Segment2DAndNormal> 
	 Engine::Cell::IsOutLine(const Vector2& EndPosition2D) const&
{
	// 2D 노말 벡터 사용해서 선분기준 노말 방향인지 반대방향인지 검사. 
	auto IsOutLineImplementation =
		[EndPosition2D](const Segment2DAndNormal& TargetSegment)
		->std::optional<Engine::Cell::Segment2DAndNormal>
	{
		const Vector2 ToEnd = EndPosition2D - TargetSegment.Begin;
		const float d = D3DXVec2Dot(&TargetSegment.Normal, &ToEnd);
		if (d > 0.0f)
		{
			return { TargetSegment };
		}
		else
		{
			return std::nullopt;
		}
	};
	// (2D Point - Segment Start Point)이 전부 다 선분 노말과 예각이여야 삼각형 내부라고 판단.
	std::optional<Engine::Cell::Segment2DAndNormal>	bOutLine = std::nullopt;

	if (bOutLine = IsOutLineImplementation(Segment2DAB);
		bOutLine)
	{
		return bOutLine;
	}

	if (bOutLine = IsOutLineImplementation(Segment2DBC);
		bOutLine)
	{
		return bOutLine;
	}

	if (bOutLine = IsOutLineImplementation(Segment2DCA);
		bOutLine)
	{
		return bOutLine;
	}

	return bOutLine;
}


