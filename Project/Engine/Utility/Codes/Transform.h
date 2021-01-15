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
			const float DeltaTime)& abstract;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Transform;
	public:
		inline const auto& GetWorld()const&;
		inline const auto& GetLocation()const&;
		inline const auto& GetRotation() const&;
		inline const auto& GetScale() const&;
		inline auto GetForward() const&;
		inline auto GetRight() const&;
		inline auto GetUp()const&;
	private:
		Matrix World;
		Vector3 Scale;
		Vector3 Rotation;
		Vector3 Location;
	};
};

inline const auto& Engine::Transform::GetWorld() const&
{
	return World;
}

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

inline auto Engine::Transform::GetForward() const&
{
	return Vector3{ World._31, World._32,World._33 };
}

inline auto Engine::Transform::GetRight() const&
{
	return Vector3{World._11 , World._12,World._13  };
}

inline auto Engine::Transform::GetUp() const&
{
	return Vector3{ World._21 , World._22,World._23 };
}



