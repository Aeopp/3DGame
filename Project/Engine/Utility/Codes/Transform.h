#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Component.h"

namespace Engine
{
	class DLL_DECL Transform : public  Component
	{
	public :
		using Super = Component;
		void Initialize()&;
		virtual void Update(class Object* const Owner,
							const float DeltaTime)& override;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Transform;
	public :
		void Rotate(Vector3 Axis, const float Radian)&;
		void Move(Vector3 Direction,const float DeltaTime,const float Speed); 
		inline void SetScale(const Vector3& Scale)&;
	public:
		const Matrix& UpdateWorld()&;
		inline const auto& GetLocation()const&;
		inline const auto& GetRotation() const&;
		inline const auto& GetScale() const&;
		inline const auto& GetForward() const&;
		inline const auto& GetRight() const&;
		inline const auto& GetUp()const&;
	private:
		void UpdateBasis(const Matrix& From)&;
	private:
		Matrix World;
		Vector3 Scale{ 1,1,1 };
		Vector3 Forward{ 0,0,1 };
		Vector3 Right{ 1,0,0 };
		Vector3 Up{ 0,1,0 };
		Vector3 Rotation{ 0,0,0 };
		Vector3 Location{ 0,0,0 };
	};
};

inline void Engine::Transform::SetScale(const Vector3& Scale)&
{
	this->Scale = Scale;
};

inline const auto& Engine::Transform::GetLocation() const&
{
	return Location; 
}

inline const auto& Engine::Transform::GetRotation() const&
{
	return Rotation; 
}

inline const auto& Engine::Transform::GetScale() const&
{
	return Scale; 
}

inline const auto& Engine::Transform::GetForward() const&
{
	return Forward;
}

inline const auto& Engine::Transform::GetRight() const&
{
	return Right;
}

inline const auto& Engine::Transform::GetUp() const&
{
	return Up;
}



