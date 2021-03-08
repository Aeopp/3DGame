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
			// Ÿ���� ��ġ�� Ʈ������ ��ġ���� �������� ���� ���� ����. 
			Vector3 TargetLocationOffset{ 0,0,0 };
			float DistancebetweenTarget{ 1.f };
			Vector3 ViewDirection{ 0.f,-0.707f,0.707f };
			float RotateResponsiveness = 1.f;
			float ZoomInOutScale = 1.f;

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
		void SetUpTarget(const TargetInformation& TargetInformationParam)&;
	public:
		float FovY{ 0.0f };
		float Far{ 1000.f };
	protected:
		bool bCursorMode{ false };
		HWND Hwnd{ NULL };
		TargetInformation _TargetInformation{};
		Vector3 Up{0,1,0};
		IDirect3DDevice9* Device{ nullptr };
		float Near{ 0.1f };
		float Aspect{};
	};
};


