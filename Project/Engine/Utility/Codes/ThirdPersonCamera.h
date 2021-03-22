#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"

namespace Engine
{
	class DLL_DECL ThirdPersonCamera: public  Object
	{
	public :
		using Super = Object;
		struct TargetInformation 
		{
			Engine::Object* TargetObject{ nullptr };
			// 타겟의 위치는 트랜스폼 위치에서 오프셋을 더한 값이 최종. 
			Vector3 TargetLocationOffset{ 0,0,0 };
			float DistancebetweenTarget{ 1.f };
			float CurrentDistancebetweenTarget = DistancebetweenTarget;
			float MaxDistancebetweenTarget{ 1000.f };
			Vector3 ViewDirection{ 0.f,-0.707f,0.707f };
			float RotateResponsiveness = 1.f;
			float ZoomInOutScale = 1.f;

			float LocationLerpSpeed = 4.f;

			Vector3 CurrentViewDirection{ 1,0,0 };
			Vector3 CurrentTargetLocation{ 0,0,0 };
		};
		virtual void Event() & override;
		virtual void Update(const float DeltaTime)& override;
		virtual void LateUpdate(const float DeltaTime) & override;
		void Initialize(const float FovY,
						const float Near,
						const float Far,
						const float Aspect)&;
		void PrototypeInitialize(IDirect3DDevice9* const Device , const HWND Hwnd)&;
		virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
		virtual std::optional<SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
		virtual std::function<SpawnReturnValue(const SpawnParam&)> PrototypeEdit()&override;

		inline Engine::ThirdPersonCamera::TargetInformation GetTargetInformation()const& {return _TargetInformation; };
		inline Engine::ThirdPersonCamera::TargetInformation& RefTargetInformation()& { return _TargetInformation; };
		void SetUpTarget(const TargetInformation& TargetInformationParam)&;
	public:
		struct ShakeInfo
		{
			float Force{ 1.f };
			Vector3 Direction{ 1,0,0 };
			float Duration{ 0.1f };
		};
		void Shake(const float Force,
			const Vector3& Direction,
			const float Duration)&;
		bool bCameraUpdate = true;
		float FovY{ 0.0f };
		float Far{ 1000.f };
		bool bCursorMode{ false };
	protected:
		std::vector<ShakeInfo> ShakeInfos{};
		Vector3 CurrentShake{ 0,0,0 };
		void CalcCurrentShake(const float DeltaTime)&;

		HWND Hwnd{ NULL };
		TargetInformation _TargetInformation{};
		Vector3 Up{0,1,0};
		IDirect3DDevice9* Device{ nullptr };
		float Near{ 0.1f };
		float Aspect{};
	};
};



