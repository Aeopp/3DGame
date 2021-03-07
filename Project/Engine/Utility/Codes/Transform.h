#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Component.h"

namespace Engine
{
	class DLL_DECL Transform : public  Component
	{
	public:
		struct EditProperty
		{
			float LocationSensitivity = 1.f;
		};
		struct TransformProperty
		{
			Vector3 Scale{ 1,1,1 };
			Vector3 Rotation{ 0,0,0 }; 
		};
	public :
		using Super = Component;
		void Initialize(const std::string& OwnerClassIdentifier)&;
		virtual void Update(class Object* const Owner,
							const float DeltaTime)& override;
		virtual void Event(class Object* const Owner)& override;
	public:
		// 자식마다 정의.
		static const inline Property TypeProperty = Property::Transform;
	public :
		void Rotate(const Vector3& Rotation, const float DeltaTime)&;
		void RotateYaw(const float Radian, const float DeltaTime)&;
		void RotateRoll(const float Radian, const float DeltaTime)&;
		void RotatePitch(const float Radian, const float DeltaTime)&;
		void RotateAxis(Vector3 Axis, const float Radian,const float DeltaTime)&;
		void Move(Vector3 Direction, const float DeltaTime,const float Speed);
		void MoveForward(const float DeltaTime ,const float Speed);
		void MoveRight(const float DeltaTime, const float Speed);
		void MoveUp(const float DeltaTime, const float Speed);
		void SetRotation(const Vector3& Rotation)&;
		inline void SetLocation(const Vector3& Location)&;
		inline void SetScale(const Vector3& Scale)&;
	public:
		const Matrix& UpdateWorld()&;
		inline const auto& GetLocation()const&;
		inline const auto& GetRotation() const&;
		inline const auto& GetScale() const&;
		inline const auto& GetForward() const&;
		inline const auto& GetRight() const&;
		inline const auto& GetUp()const&;
	public:
		void Save()&;
		void Load()&;
	public:
		void AttachBone(const Matrix* const TargetBoneToRoot)&;
		void AttachTransform(const Matrix* const TargetParentTransform)&;
	private:
		EditProperty _EditProperty{};
		const Matrix* AttachBoneToRoot = nullptr;
		const Matrix* OwnerTransform = nullptr;
		std::string OwnerClassIdentifier{};
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

inline void Engine::Transform::SetLocation(const Vector3& Location)&
{
	this->Location = Location;
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

//inline auto& Engine::Transform::RefLocation()&
//{
//	return Location;
//}
//inline auto& Engine::Transform::RefRotation()&
//{
//	return Rotation;
//}
//inline auto& Engine::Transform::RefScale()&
//{
//	return Scale; 
//};


