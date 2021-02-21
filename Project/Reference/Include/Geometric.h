#pragma once
#include "DllHelper.H"
#include <type_traits>
#include "TypeAlias.h"
#include <typeinfo>
#include <array>
#include <optional>
#include "MathStruct.h"

namespace Engine
{
	class DLL_DECL Geometric
	{
	protected:
		enum class Type : uint8
		{
			None,
			OBB,
			Sphere,
		};
	protected:
		Geometric(const float Radius, const Vector3 Center) : LocalSphere{Radius,Center}  {};
	public:
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs)& abstract;
		virtual void Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location,
			const Matrix& OffsetMatrix, const Vector3& OffsetScale)& abstract;
		virtual void Update(const Matrix& ToOwnerWorld, const Vector3& OwnerScale, const Matrix& OffsetMatrix, const Vector3& OffsetScale)& abstract;
		virtual Type GetType() const& abstract;
		virtual void Render(IDirect3DDevice9* const Device, const bool bCurrentUpdateCollision)& abstract;
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
		IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
		IDirect3DTexture9* CollisionTexture{ nullptr };
		IDirect3DTexture9* NoCollisionTexture{ nullptr };
		const Sphere LocalSphere;
		Sphere WorldSphere;
	};

	class DLL_DECL GSphere : public Geometric
	{
	public:
		GSphere(const float Radius,const Vector3 Center);
		void MakeDebugCollisionSphere(IDirect3DDevice9* const Device);
		virtual Type GetType() const& override;
		virtual void Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location,
			const Matrix& OffsetMatrix, const Vector3& OffsetScale) & override;
		virtual void Update(const Matrix& ToOwnerWorld, const Vector3& OwnerScale, const Matrix& OffsetMatrix, const Vector3& OffsetScale)& override;
		virtual void Render(IDirect3DDevice9* const Device, const bool bCurrentUpdateCollision) & override;
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs) & override;
		std::optional<std::pair<float, Vector3>> IsCollisionOBB(class OBB* const Rhs)const&;
		std::optional<std::pair<float, Vector3>> IsCollisionSphere(GSphere* const Rhs)const&;
		ID3DXMesh* _SphereMesh{ nullptr };
	};

	class DLL_DECL OBB : public Geometric
	{
	public:
		OBB(const Vector3 LocalMin, const Vector3 LocalMax);
		void MakeDebugCollisionBox(IDirect3DDevice9* const Device);
		virtual Type GetType() const& override;
		virtual void Update(const Matrix& ToOwnerWorld, const Vector3& OwnerScale, const Matrix& OffsetMatrix, const Vector3& OffsetScale) & override;
		virtual void Update(const Vector3 Scale, const Vector3 Rotation, const Vector3 Location,
			const Matrix& OffsetMatrix, const Vector3& OffsetScale) & override;
		virtual void Render(IDirect3DDevice9* const Device , const bool bCurrentUpdateCollision) & override;
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs) & override;
		std::optional<std::pair<float,Vector3>> IsCollisionOBB(OBB* const Rhs)const&;
		std::optional<std::pair<float, Vector3>>IsCollisionSphere(GSphere* const Rhs)const&;

		std::array<Vector3,8u> WorldPoints;
		Vector3 WorldCenter; 
		Vector3 WorldHalfDistances{};
		std::array<Vector3,3u> WorldFaceNormals;
		const std::array<Vector3,8u> LocalPoints;
		const std::array<Vector3,3u> LocalFaceNormals;
		const Vector3 LocalCenter;
		const Vector3 LocalHalfDistances{};
	};
}