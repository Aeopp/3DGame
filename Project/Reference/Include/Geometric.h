#pragma once
#include "DllHelper.H"
#include <type_traits>
#include "TypeAlias.h"
#include <typeinfo>
#include <array>

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
	public:
		virtual bool IsCollision(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale)& abstract;
		virtual void Update(const Vector3 Scale,
							const Vector3 Rotation,
							const Vector3 Location)& abstract;
		virtual Type GetType() const& abstract;
		virtual void Render(IDirect3DDevice9* const Device, const bool bCurrentUpdateCollision)& abstract;
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
		IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
		IDirect3DTexture9* CollisionTexture{ nullptr };
		IDirect3DTexture9* NoCollisionTexture{ nullptr };
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
		virtual bool IsCollision(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale) & override;
		bool IsCollisionAABB(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale)const&;
		bool IsCollisionOBB(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale)const&;
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
		virtual bool IsCollision(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale) & override;
		bool IsCollisionOBB(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale)const&;
		bool IsCollisionAABB(Geometric* const Rhs,
			Vector3& PushDir,
			float& CrossAreaScale)const&;
		std::array<Vector3, 8u> WorldPoints;
		Vector3 WorldCenter; 
		std::array<Vector3, 3u> WorldFaceNormals;
		/*float DistanceAxisX;
		float DistanceAxisY;
		float DistanceAxisZ;*/
		const std::array<Vector3, 8u> LocalPoints;
		const std::array<Vector3, 3u > LocalFaceNormals;
		const Vector3 LocalCenter;
	};

}