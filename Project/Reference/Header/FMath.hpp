#pragma once
#include "TypeAlias.h"
#include <type_traits>
#include <numeric>
#include <numbers>
#include <array>
#include <d3d9.h>
#include <d3dx9.h>
#include <random>
#include "MathStruct.h"


class FMath
{
public:
	static constexpr auto PI = std::numbers::pi_v<float>;

	template<class T>
	typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
	static	inline AlmostEqual(T x, T y);
   
	static inline float Dot(const Vector3& Lhs, const Vector3& Rhs);
	static inline Vector3 Cross(const Vector3& Lhs, const Vector3& Rhs);
	static inline Vector3 Normalize(const Vector3& Lhs);
    static inline Vector3 GetNormalFromFace(const Vector3& p0,
		const Vector3& p1, const Vector3& p2);

	static inline Vector4 ConvertVector4(const Vector3& Lhs, const float w);;
	static inline Vector3 Mul(const Vector3& Lhs, const Matrix& Rhs);
	static inline Vector3 MulNormal(const Vector3& Lhs, const Matrix& Rhs);

	static inline Ray GetRayScreenProjection(const Vector3& ScreenPos,
		IDirect3DDevice9* const _Device, const float Width, const float Height);
	static inline bool InnerPointFromFace(const Vector3& Point, const std::array<Vector3, 3ul>& Face);

	// 임의의 위치 벡터를 평면에 투영시킨 위치 벡터를 반환.
	static inline Vector3 ProjectionPointFromFace(const D3DXPLANE _Plane, const Vector3& Point);

	// 삼각형을 둘러치는 선분 3개.
	static inline std::array<Segment, 3ul>
			MakeSegmentFromFace(const std::array<Vector3, 3ul>& Face);
	
	static inline float ToRadian(const float Degree);
	static inline float ToDegree(const float Radian);
	
	static inline Vector3 RotationVecCoord(const Vector3& Lhs,
					const Vector3& Axis, const float Radian);
	static inline Vector3 RotationVecNormal(const Vector3& Lhs,
		const Vector3& Axis, const float Radian);
	template<typename _Ty>
	static inline _Ty Lerp(const _Ty& Lhs, const _Ty& Rhs, const float t);
	static inline float Length(const Vector3& Lhs);
	static inline float LengthSq(const Vector3& Lhs);;
	static inline float LengthSq(const Vector4& Lhs);;
	static inline Matrix Inverse(const Matrix& _Matrix);
	static inline Matrix Transpose(const Matrix& _Matrix);
	template<typename T>
	static inline T Clamp(const T& Target, const T& Min, const T& Max);;

	static inline Matrix RotationAxisMatrix(
		const Vector3 Axis, const float Radian);;

	static inline Matrix Identity(Matrix& _Matrix);

	static inline Matrix WorldMatrix(
									const Vector3& Scale, 
									const Vector3& Forward,
									const Vector3& Right,
									const Vector3& Up,
									const Vector3& Location);

	static inline Matrix WorldMatrix(const Vector3& Scale, const Vector3& Rotation,
								const Vector3& Location);

	static inline Matrix Scale(const Vector3& Scale);
						// Yaw Pitch Roll
	static inline Matrix Rotation(const Vector3& Rotation);
	static inline Matrix Translation(const Vector3& Location);


	static inline bool IsSphereToSphere(const Sphere& Lhs, const Sphere& Rhs,
		float& CrossingArea,
		Vector3& IntersectPointLhs,
		Vector3& IntersectPointRhs);

	static inline bool IsRayToSphere(
		const Ray& Lhs, const Sphere& Rhs,
		float& t0, float& t1, Vector3& OutIntersectPoint);

	static inline bool
		IsSegmentToSphere(
			const Segment& Lhs, const Sphere& Rhs,
			float& t0, float& t1, Vector3& IntersectPoint);

	static inline bool  IsPlaneToSphere(const PlaneInfo& Lhs,
								const Sphere& Rhs, float& CrossingArea);


	static inline bool IsTriangleToRay(
		const PlaneInfo& Lhs,
		const Ray& Rhs,
		float& t, Vector3& IntersectPoint);

#pragma region RANDOM
	template<typename Type>
	static inline Type Random(const Type& Begin, const Type& End);
private:
	static inline auto& GetGenerator();
#pragma endregion RANDOM
};

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
FMath::AlmostEqual(T x, T y)
{
	return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y)
		|| std::fabs(x - y) < (std::numeric_limits<T>::min)();
};

template<typename Type>
 Type FMath::Random(const Type& Begin, const Type& End)
{
	if constexpr (std::is_floating_point_v<Type>)
	{
		std::uniform_real_distribution<Type> Dis(Begin, End);
		return Dis(GetGenerator());
	}
	else if constexpr (std::is_same_v<Type, Vector3>)
	{
		return Vector3{
		Random(Begin.x,End.x),
		Random(Begin.y,End.y),
		Random(Begin.z,End.z) };
	}
	else
	{
		std::uniform_int_distribution<Type> Dis(Begin, End);
		return Dis(GetGenerator());
	}
};

auto& FMath::GetGenerator()
{
	static bool bInit = false;
	static std::random_device Rd{};
	static std::mt19937 gen{};

	if (!bInit)
	{
		bInit = true;
		gen.seed(Rd());
	}

	return gen;
};


inline Vector4 FMath::ConvertVector4(const Vector3& Lhs, const float w)
{
	return Vector4{ Lhs.x, Lhs.y, Lhs.z, w };
}

Vector3 FMath::Mul(const Vector3& Lhs, const Matrix& Rhs)
{
	Vector3 TransformVec;
	return *D3DXVec3TransformCoord(&TransformVec, &Lhs, &Rhs);;
}

Vector3 FMath::MulNormal(const Vector3& Lhs, const Matrix& Rhs)
{
	return *D3DXVec3TransformNormal(nullptr, &Lhs, &Rhs);;
}

inline  Vector3 FMath::GetNormalFromFace(const Vector3& p0,
								const Vector3& p1, const Vector3& p2)
{
	const Vector3 u = p1 - p0;
	const Vector3 v = p2 - p0;
	return Normalize(Cross(u, v));
}

Ray FMath::GetRayScreenProjection(
	const Vector3& ScreenPos, 
	IDirect3DDevice9* const _Device, 
	const float Width, 
	const float Height)
{
	Vector3 Dir{ (ScreenPos.x),(ScreenPos.y),1.f };
	const float xfactor = (2.f / Width);
	const float yfactor = -(2.f / Height);
	Dir.x = Dir.x * xfactor - 1.f;
	Dir.y = Dir.y * yfactor + 1.f;
	Matrix Proj, InvView;
	_Device->GetTransform(D3DTS_PROJECTION, &Proj);
	Dir.x /= Proj(0, 0);
	Dir.y /= Proj(1, 1);
	Dir = FMath::Normalize(Dir);
	_Device->GetTransform(D3DTS_VIEW, &InvView);
	InvView = FMath::Inverse(InvView);
	Dir = FMath::MulNormal(Dir, InvView);
	Vector3 Origin = { InvView(3,0),InvView(3,1),InvView(3,2) };
	Ray _Ray;
	_Ray.Start = std::move(Origin);
	_Ray.Direction = std::move(Dir);
	return _Ray;
}


inline  float FMath::Dot(const Vector3& Lhs, const Vector3& Rhs)
{
	return D3DXVec3Dot(&Lhs, &Rhs);
}

inline Vector3 FMath::Cross(const Vector3& Lhs, const Vector3& Rhs)
{
	Vector3 Target;
	return *D3DXVec3Cross(&Target, &Lhs, &Rhs);;
}

inline Vector3 FMath::Normalize(const Vector3& Lhs)
{
	Vector3 Target;
	 D3DXVec3Normalize(&Target, &Lhs);
	 return Target;
}

Vector3 FMath::ProjectionPointFromFace(const D3DXPLANE _Plane, const Vector3& Point)
{
	Vector3 Normal = { _Plane.a,_Plane.b,_Plane.c };
	Normal = FMath::Normalize(Normal);
	const float distance = FMath::Dot(Normal, Point) + _Plane.d;
	Normal = -Normal;
	return Point + (Normal * distance);
}

std::array<Segment, 3ul> FMath::MakeSegmentFromFace(const std::array<Vector3, 3ul>& Face)
{
	std::array<Segment, 3ul> Segments;

	{
		Segments[0]._Ray.Start = Face[0];
		const Vector3 Distance = Face[1] - Face[0];
		Segments[0]._Ray.Direction = FMath::Normalize(Distance);
		Segments[0].t = FMath::Length(Distance);
	}

	{
		Segments[1]._Ray.Start = Face[1];
		const Vector3 Distance = Face[2] - Face[1];
		Segments[1]._Ray.Direction = FMath::Normalize(Distance);
		Segments[1].t = FMath::Length(Distance);
	}

	{
		Segments[2]._Ray.Start = Face[2];
		const Vector3 Distance = Face[0] - Face[2];
		Segments[2]._Ray.Direction = FMath::Normalize(Distance);
		Segments[2].t = FMath::Length(Distance);
	}

	return Segments;
}

inline float FMath::ToRadian(const float Degree) { return Degree * (PI / 180.0f); }

inline float FMath::ToDegree(const float Radian) { return Radian * (180.0f / PI); }

inline float FMath::Length(const Vector3& Lhs)
{
	return D3DXVec3Length(&Lhs);
}

inline float FMath::LengthSq(const Vector3& Lhs)
{
	return D3DXVec3LengthSq(&Lhs);
}

inline float FMath::LengthSq(const Vector4& Lhs)
{
	return D3DXVec4LengthSq(&Lhs);
}

inline Matrix FMath::Inverse(const Matrix& _Matrix)
{
	Matrix InverseMatrix;
	return *D3DXMatrixInverse(&InverseMatrix, nullptr, &_Matrix);;
}

inline Matrix FMath::Transpose(const Matrix& _Matrix)
{
	Matrix TransposeMatrix;
	return *D3DXMatrixTranspose(&TransposeMatrix, &_Matrix);;
}

Vector3 FMath::RotationVecCoord(const Vector3& Lhs, const Vector3& Axis,
	const float Radian)
{
	Vector3 Target{};
	Matrix TargetMatrix{};
	return *D3DXVec3TransformCoord(
		&Target, &Lhs, D3DXMatrixRotationAxis(&TargetMatrix, &Axis, Radian));;
}

Vector3 FMath::RotationVecNormal(const Vector3& Lhs, const Vector3& Axis,
	const float Radian)
{
	Vector3 Target{};
	Matrix TargetMatrix{};
	return *D3DXVec3TransformNormal(&Target, &Lhs,
		D3DXMatrixRotationAxis(&TargetMatrix, &Axis, Radian));;
};

inline Matrix FMath::Scale(const Vector3& Scale)
{
	Matrix Target;
	return *D3DXMatrixScaling(&Target, Scale.x, Scale.y, Scale.z);
}
inline Matrix FMath::Rotation(const Vector3& Rotation)
{
	Matrix Target;
	return *D3DXMatrixRotationYawPitchRoll(&Target, Rotation.y, Rotation.x, Rotation.z);
}
inline Matrix FMath::Translation(const Vector3& Location)
{
	Matrix Target;
	return *D3DXMatrixTranslation(&Target, Location.x, Location.y, Location.z);
}

inline Matrix FMath::Identity(Matrix& _Matrix)
{
	return *D3DXMatrixIdentity(&_Matrix);
};

 inline Matrix FMath::WorldMatrix(
	const Vector3& Scale,
	const Vector3& Forward,
	const Vector3& Right,
	const Vector3& Up,
	const Vector3& Location)
{
	static constexpr size_t Vec3Size = sizeof(Vector3); 
	Matrix BasisMatrix;
	BasisMatrix=FMath::Identity(BasisMatrix);
	memcpy(&BasisMatrix._11, &Right, Vec3Size);
	memcpy(&BasisMatrix._21, &Up, Vec3Size);
	memcpy(&BasisMatrix._31, &Forward, Vec3Size);
	return 	FMath::Scale(Scale)* BasisMatrix* FMath::Translation(Location);
};

Matrix FMath::WorldMatrix(
	const Vector3& Scale, const Vector3& Rotation, const Vector3& Location)
{
	return FMath::Scale(Scale) * FMath::Rotation(Rotation) * FMath::Translation(Location);
}

template<typename _Ty>
inline _Ty FMath::Lerp(const _Ty& Lhs, const _Ty& Rhs, const float t)
{
	return Lhs + t * (Rhs - Lhs);
}


template<typename T>
inline T FMath::Clamp(const T& Target, const T& Min, const T& Max)
{
	if (Target < Min)
	{
		return Min;
	}
	else if (Target > Max)
	{
		return Max;
	}

	return Target;
}


inline Matrix FMath::RotationAxisMatrix(const Vector3 Axis, const float Radian)
{
	Matrix RotationMatrix;
	return *D3DXMatrixRotationAxis(&RotationMatrix, &Axis, Radian);;
}

inline bool FMath::IsTriangleToRay(
	const PlaneInfo& Lhs,
	const Ray& Rhs,
	float& t, Vector3& IntersectPoint)
{
	const Vector3 Normal = FMath::Normalize({ Lhs._Plane.a ,
												Lhs._Plane.b,
												Lhs._Plane.c });

	const float Dot = FMath::Dot(Normal, Rhs.Direction);

	if (FMath::AlmostEqual(Dot, 0.f))
	{
		return false;
	}

	float NQ_D = -(FMath::Dot(Normal, Rhs.Start) + Lhs._Plane.d);

	t = NQ_D / Dot;

	if (t < 0)
	{
		return false;
	};

	IntersectPoint = Rhs.Start + Rhs.Direction * t;

	return FMath::InnerPointFromFace(IntersectPoint, Lhs.Face);
};


bool FMath::IsSegmentToSphere(const Segment& Lhs, const Sphere& Rhs,
	float& t0, float& t1, Vector3& IntersectPoint)
{
	const bool bCollision = IsRayToSphere(Lhs._Ray, Rhs, t0, t1, IntersectPoint);

	if (false == bCollision)return false;

	return (((t0 >= 0) && (t0 <= Lhs.t) && bCollision) ||
		((t1 >= 0) && (t1 <= Lhs.t) && bCollision));
};

bool FMath::IsPlaneToSphere(
	const PlaneInfo& Lhs,
	const Sphere& Rhs,
	float& CrossingArea)
{
	Vector3 Normal = { Lhs._Plane.a, Lhs._Plane.b,Lhs._Plane.c };
	const float d = Lhs._Plane.d;
	const float Fomula = FMath::Dot(Normal, Rhs.Center) + d;
	Vector3 ToPlaneCenter = Lhs.Center - Rhs.Center;

	if (Fomula <= Rhs.Radius && Fomula >= 0)
	{		
		CrossingArea = Rhs.Radius - Fomula;
		return true;
	}

	return false;
};


 bool FMath::IsSphereToSphere(const Sphere& Lhs, const Sphere& Rhs,
	float& CrossingArea,
	Vector3& IntersectPointLhs,
	Vector3& IntersectPointRhs)
{
	Vector3 ToRhs = Rhs.Center - Lhs.Center;
	const float Distance = FMath::Length(ToRhs);
	const float RadiusSum = Lhs.Radius + Rhs.Radius;

	if (Distance < RadiusSum)
	{
		Vector3 ToLhs = Normalize(Lhs.Center - Rhs.Center);
		ToRhs = Normalize(ToRhs);
		CrossingArea = RadiusSum - Distance;
		IntersectPointRhs = Rhs.Center + Rhs.Radius * ToLhs;
		IntersectPointLhs = Lhs.Center + Lhs.Radius * ToRhs;
		return true;
	}

	return false;
};

 bool FMath::IsRayToSphere(
	 const Ray& Lhs, const Sphere& Rhs,
	 float& t0, float& t1, Vector3& OutIntersectPoint)
 {
	 const Vector3 RayStartToCenter = Rhs.Center - Lhs.Start;
	 const float dot = FMath::Dot(Lhs.Direction, RayStartToCenter);
	 const Vector3 IntersectPoint = Lhs.Start + Lhs.Direction * dot;
	 Vector3 ItPtToCenter = IntersectPoint - Rhs.Center;
	 const float Distance = FMath::Length(ItPtToCenter);
	 const float m = FMath::Length(IntersectPoint - Rhs.Center);
	 const float q = std::sqrtf((Rhs.Radius * Rhs.Radius) - (m * m));
	 t0 = dot - q;
	 t1 = dot + q;
	 if (Distance < Rhs.Radius && (t0 >= 0 || t1 >= 0))
	 {
		 OutIntersectPoint = IntersectPoint;
		 return true;
	 }

	 return false;
 }

 inline bool FMath::InnerPointFromFace(const Vector3& Point, const std::array<Vector3, 3ul>& Face)
 {
	 std::array <Vector3, 3u> ToVertexs;

	 for (size_t i = 0; i < ToVertexs.size(); ++i)
	 {
		 ToVertexs[i] = FMath::Normalize(Face[i] - Point);
	 }

	 float Radian = 0;
	 Radian += std::acosf(Dot(ToVertexs[0], ToVertexs[1]));
	 Radian += std::acosf(Dot(ToVertexs[1], ToVertexs[2]));
	 Radian += std::acosf(Dot(ToVertexs[2], ToVertexs[0]));

	 return AlmostEqual<float>(Radian, PI * 2.f);

 }
