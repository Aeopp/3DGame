#pragma once
#include "TypeAlias.h"
#include <array>

struct Sphere
{
	float Radius;
	Vector3 Center;
};

struct Ray
{
	Vector3 Start;
	Vector3 Direction;
};

struct Ray2D
{
	Vector2 Start;
	Vector2 Direction;
};

struct Segment2D
{
	static Segment2D Make(const Vector2& Begin, const Vector2& End)
	{
		Segment2D _Segment;
		_Segment.End = End;
		_Segment._Ray.Start = Begin;
		Vector2 ToEnd = End - Begin;
		_Segment.t = D3DXVec2Length(&ToEnd);
		_Segment._Ray.Direction = *D3DXVec2Normalize(nullptr, &ToEnd);
		return _Segment;
	};

	Ray2D _Ray{};
	Vector2 End{};
	float t{ 0.f };
};

struct Segment
{
	static Segment Make(const Vector3& Begin, const Vector3& End)
	{
		Segment _Segment;
		_Segment.End = End;
		_Segment._Ray.Start = Begin;
		Vector3 ToEnd = End - Begin;
		_Segment.t = D3DXVec3Length(&ToEnd);
		_Segment._Ray.Direction = *D3DXVec3Normalize(nullptr, &ToEnd);
		return _Segment;
	};

	Ray _Ray{};
	Vector3 End{};
	float t{0.f};
};

struct PlaneInfo
{
	static PlaneInfo Make(std::array<Vector3, 3ul> Face)
	{
		PlaneInfo _Info;
		_Info.Face = std::move(Face);
		_Info.Center = (Face[0] + Face[1] + Face[2]) / (float)Face.size();
		D3DXPlaneFromPoints(&_Info._Plane,
			&_Info.Face[0], &_Info.Face[1], &_Info.Face[2]);
		return _Info;
	};

	D3DXPLANE _Plane;
	Vector3 Center;
	// 로컬이 아닌 월드 기준
	std::array<Vector3, 3ul> Face;
};