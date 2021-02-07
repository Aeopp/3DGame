#pragma once
#include "TypeAlias.h"
#include "MathStruct.h"
#include "DllHelper.H"

namespace Engine
{
	class DLL_DECL Cell
	{
		struct Segment2DAndNormal
		{
			static Segment2DAndNormal Make(const Vector2 Begin, const Vector2 End);;
			Vector2 Begin; 
			Vector2 End; 
			Vector2 ToEnd{}; 
			Vector2 Normal; 
		};
	public:
		enum class CompareType :uint8  {Moving,Stop,End};
	public:
		void Initialize(
			class NavigationMesh* const NaviMesh,
			const uint32 Index,
			const Vector3& PointA,
			const Vector3& PointB,
			const Vector3& PointC ,
			IDirect3DDevice9* const Device)&;
		bool FindNeighbor(const Vector3& PointFirst,const Vector3& PointSecond,
			Cell* _Cell)&;
		void Render(IDirect3DDevice9* Device)&;
		// 상태와 새로운 인덱스를 반환. 이동할수 없는 상태일경우 인덱스는 유효하지 않음.
		std::pair<Engine::Cell::CompareType,uint32> 
			Compare(const Vector3& EndPosition, const uint32 CellIndex)const&;
	public:
		/*
		A->B 
		B->C 
		C->A
		*/
		Vector3 PointA{ 0,0,0 };
		Vector3 PointB{ 0,0,0 };
		Vector3 PointC{ 0,0,0 };
		// Neighbor.
		Cell* NeighborAB{ nullptr };
		Cell* NeighborBC{ nullptr };
		Cell* NeighborCA{ nullptr };
		Segment2DAndNormal Segment2DAB{};
		Segment2DAndNormal Segment2DBC{};
		Segment2DAndNormal Segment2DCA{};
		ID3DXMesh* DebugSphereMesh{ nullptr };
		ID3DXLine* Line{ nullptr };
		uint32 Index{ 0u };
	};
};




inline Engine::Cell::Segment2DAndNormal 
Engine::Cell::Segment2DAndNormal::Make(const Vector2 Begin, const Vector2 End)
{
	Segment2DAndNormal _Target;
	_Target.Begin = Begin;
	_Target.End = End;
	_Target.ToEnd = End - Begin;
	_Target.Normal = (Vector2(_Target.ToEnd.y * -1.f, _Target.ToEnd.x));
	D3DXVec2Normalize(&_Target.Normal, &_Target.Normal);
	return _Target;
};