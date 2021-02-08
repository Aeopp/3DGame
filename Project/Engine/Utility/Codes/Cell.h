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
			const Vector3& PointA,
			const Vector3& PointB,
			const Vector3& PointC ,
			IDirect3DDevice9* const Device)&;
		bool FindNeighbor(const Vector3& PointFirst,const Vector3& PointSecond,
			Engine::Cell* _Cell)&;
		void Render(IDirect3DDevice9* Device)&;
		// ���¿� ���ο� �ε����� ��ȯ. �̵��Ҽ� ���� �����ϰ�� �ε����� ��ȿ���� ����.
		std::pair<Engine::Cell::CompareType,const Engine::Cell*> 
			Compare(const Vector3& EndPosition)const&;
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