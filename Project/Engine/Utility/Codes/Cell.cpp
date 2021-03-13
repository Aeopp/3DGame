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
		// �ش� ���� �ٱ����� ���� ����̹Ƿ� 
		// �ش� ���а� �̿��� ���� �̿��� �����ϴ��� �˻��� ���Ŀ�
		// �̿��� ���ٸ� �̵� �Ұ�. �̿��� �����Ѵٸ� �̿����� �� ��ü �� ���Ŀ� �̵�. 
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


		//// �̿��鿡���� �̵��� ���� �Ͽ����Ƿ� �̿��� �̿����� �˻� ����. (��� ����)
		//for (const auto& NeighborCell : Neighbors)
		//{
		//	auto NeighborResult = NeighborCell->Compare(EndPosition);
		//	if  (NeighborResult)
		//	{
		//		return NeighborResult;
		//	}
		//}

		// �̿��� �߿����� �˻����� �����Ͽ���.
		Vector3 Force = EndPosition - Vector3{ bOutLine->Begin.x , 0.f , bOutLine->Begin.y };
		Vector3 OutlineSegmentSlidingVector =FMath::Sliding(Force, { bOutLine->Normal.x ,0.f,bOutLine->Normal.y} );
		Force.y = 0.0f;

		const Vector3 ToEnd3D = { bOutLine->ToEnd.x  , 0.f , bOutLine->ToEnd.y  };
		if (FMath::Length(OutlineSegmentSlidingVector) >= FMath::Length(ToEnd3D))
		{
			OutlineSegmentSlidingVector = ToEnd3D;
		}

		// Begin + �����̵� �����̹Ƿ� Ÿ���� �̲����� ������ ��ġ .
		Vector3 SlidingAfterLocation = OutlineSegmentSlidingVector + Vector3{ bOutLine->Begin.x , 0.f , bOutLine->Begin.y };
		// �� �ﰢ���� �������� Y Axis ��ġ�� ���ϱ� ���� �븻 �ݴ�� ���� ��¦ �о �ݵ�� �� �ﰢ�� ���ο� ��ġ�ϵ��� �Ұ��� .
		// Vector2 InnerTriangleCorrectionVector = (bOutLine->Normal  *0.000001f);
		// SlidingAfterLocation.x -= InnerTriangleCorrectionVector.x;
		// SlidingAfterLocation.z -= InnerTriangleCorrectionVector.y;
		SlidingAfterLocation.y = EndPosition.y;
		SlidingAfterLocation=FMath::ProjectionPointFromFace(_Plane._Plane, SlidingAfterLocation);
		return  { { nullptr,Engine::Cell::CompareType::Stop, SlidingAfterLocation} };
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

 std::optional<Engine::Cell::Segment2DAndNormal> Engine::Cell::IsOutLine(const Vector2& EndPosition2D) const&
{
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


