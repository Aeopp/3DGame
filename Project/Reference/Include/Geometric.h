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
			AABB,
			OBB,
		};
	protected:
		Geometric(const float Radius, const Vector3 Center) : LocalSphere{Radius,Center}  {};
	public:
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs)& abstract;
		virtual void Update(const Vector3 Scale,
							const Vector3 Rotation,
							const Vector3 Location)& abstract;
		virtual Type GetType() const& abstract;
		virtual void Render(IDirect3DDevice9* const Device, const bool bCurrentUpdateCollision)& abstract;
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
		IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
		IDirect3DTexture9* CollisionTexture{ nullptr };
		IDirect3DTexture9* NoCollisionTexture{ nullptr };
		const Sphere LocalSphere;
		Sphere WorldSphere;
	};

	class DLL_DECL AABB : public Geometric
	{
	public:
		AABB(const Vector3 LocalMin, const Vector3 LocalMax);
		virtual Type GetType() const& override;
		void MakeDebugCollisionBox(IDirect3DDevice9* const Device)&;
		virtual void Update(const Vector3 Scale,
			const Vector3 Rotation,
			const Vector3 Location) & override;
		virtual void Render(IDirect3DDevice9* const Device , const bool bCurrentUpdateCollision) & override;
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs) & override;
		std::optional<std::pair<float, Vector3>> IsCollisionAABB(Geometric* const Rhs)const&;
		std::optional<std::pair<float, Vector3>> IsCollisionOBB(Geometric* const Rhs)const&;

		Vector3 Min;
		Vector3 Max;
		const Vector3 LocalMax;
		const Vector3 LocalMin;
	};
	class DLL_DECL OBB : public Geometric
	{
	public:
		OBB(const Vector3 LocalMin, const Vector3 LocalMax);
		void MakeDebugCollisionBox(IDirect3DDevice9* const Device);
		virtual Type GetType() const& override;
		virtual void Update(const Vector3 Scale,
			const Vector3 Rotation,
			const Vector3 Location) & override;
		virtual void Render(IDirect3DDevice9* const Device , const bool bCurrentUpdateCollision) & override;
		virtual std::optional<std::pair<float, Vector3>> IsCollision(Geometric* const Rhs) & override;
		std::optional<std::pair<float,Vector3>> IsCollisionOBB(Geometric* const Rhs)const&;
		std::optional<std::pair<float, Vector3>> IsCollisionAABB(Geometric* const Rhs)const&;
		std::array<Vector3, 8u> WorldPoints;
		Vector3 WorldCenter; 
		std::array<Vector3, 3u> WorldFaceNormals;
		const std::array<Vector3, 8u> LocalPoints;
		const std::array<Vector3, 3u > LocalFaceNormals;
		const Vector3 LocalCenter;
	};

}