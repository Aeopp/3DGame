#include "ThirdPersonCamera.h"
#include "Transform.h"
#include "imgui.h"
#include "FMath.hpp"
#include "Controller.h"
#include "UtilityGlobal.h"

void Engine::ThirdPersonCamera::Initialize(
	const float FovY,
	const float Near,
	const float Far,
	const float Aspect
)&
{
	Super::Initialize();
	this->FovY = FovY;
	this->Near = Near;
	this->Far = Far;
	this->Aspect = Aspect;
};


void Engine::ThirdPersonCamera::Event()&
{
	Super::Event();

	// ShowCursor(!bCursorMode);

	if (Engine::Global::bDebugMode)
	{
		ImGui::SliderFloat3("TargetLocationOffset",_TargetInformation .TargetLocationOffset, 0.0f, 1000.f);
		ImGui::SliderFloat("RotateResponsiveness", &_TargetInformation.RotateResponsiveness, 0.0f, 1.f);
		ImGui::SliderFloat("ZoomInOutScale",       &_TargetInformation.ZoomInOutScale, 0.0f, 1.f);
	}

	auto& _Control = Controller::Instance;

	if (_Control->IsDown(DIK_P))
	{
		bCursorMode = !bCursorMode;
	}
}

void Engine::ThirdPersonCamera::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	if (!bCameraUpdate)return;

	if (bCursorMode==false)
	{
		auto& _Control = Controller::Instance;

		const float RotateHead = _Control->GetMouseMove(Engine::MouseMove::X) * _TargetInformation.RotateResponsiveness;
		_TargetInformation.ViewDirection = FMath::Normalize(FMath::RotationVecNormal(_TargetInformation.ViewDirection, { 0,1,0 }, RotateHead));

		const Vector3 CameraRight = FMath::Normalize(FMath::Cross(Vector3{ 0,1,0 }, _TargetInformation.ViewDirection));

		const float RotateRight = _Control->GetMouseMove(Engine::MouseMove::Y) *     _TargetInformation.RotateResponsiveness;
		_TargetInformation.ViewDirection = FMath::Normalize(FMath::RotationVecNormal(_TargetInformation.ViewDirection, CameraRight, RotateRight));
		
		const float ZDelta = -_Control->GetMouseMove(Engine::MouseMove::Z) * _TargetInformation.ZoomInOutScale;
		_TargetInformation.DistancebetweenTarget += ZDelta;

		D3DVIEWPORT9 ViewPort;
		Device->GetViewport(&ViewPort);
		POINT FixPosition{ static_cast<int32>(ViewPort.Width / 2u) ,
							static_cast<int32>(ViewPort.Height / 2u) };
		ClientToScreen(Hwnd, &FixPosition);
		SetCursorPos(FixPosition.x, FixPosition.y);
	}
};


void Engine::ThirdPersonCamera::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
	if (!bCameraUpdate)return;

	if (_TargetInformation.TargetObject == nullptr)return;

	if (_TargetInformation.DistancebetweenTarget > _TargetInformation.MaxDistancebetweenTarget)
	{
		_TargetInformation.DistancebetweenTarget= 
			FMath::Lerp(_TargetInformation.DistancebetweenTarget, _TargetInformation.MaxDistancebetweenTarget,
				DeltaTime* (1.f / 4.f));
	}

	_TargetInformation.CurrentDistancebetweenTarget =
		FMath::Lerp(_TargetInformation.CurrentDistancebetweenTarget, _TargetInformation.DistancebetweenTarget,
			DeltaTime * (1.f));

	Matrix View;
	Matrix Projection;

	const Vector3 TargetLocation =
		_TargetInformation.TargetObject->GetComponent<Engine::Transform>()->GetLocation() + _TargetInformation.TargetLocationOffset;

	_TargetInformation.CurrentTargetLocation =
		FMath::Lerp(_TargetInformation.CurrentTargetLocation, TargetLocation, DeltaTime * _TargetInformation.LocationLerpSpeed);

	_TargetInformation.CurrentViewDirection = FMath::Normalize(
		FMath::Lerp(_TargetInformation.CurrentViewDirection, _TargetInformation.ViewDirection, DeltaTime * 2.5f));

	const Vector3 EyeLocation = _TargetInformation.CurrentTargetLocation +
		(-_TargetInformation.CurrentViewDirection * _TargetInformation.CurrentDistancebetweenTarget);

	static const Vector3 Up = { 0,1,0 };
	D3DXMatrixLookAtLH(&View, &EyeLocation, &TargetLocation, &Up);
	D3DXMatrixPerspectiveFovLH(&Projection, FovY, Aspect, Near, Far);

	Device->SetTransform(D3DTS_VIEW, &View);
	Device->SetTransform(D3DTS_PROJECTION, &Projection);
};

void Engine::ThirdPersonCamera::PrototypeInitialize(IDirect3DDevice9* const Device , const HWND Hwnd)&
{
	Super::PrototypeInitialize();

	bCapturable = false;

	this->Hwnd = Hwnd;
	this->Device = Device;
}
std::shared_ptr<Engine::Object> Engine::ThirdPersonCamera::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)> Clone = *this;
	return std::make_shared<Engine::ThirdPersonCamera>(Clone);
}
std::optional<Engine::Object::SpawnReturnValue> Engine::ThirdPersonCamera::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	return std::optional<SpawnReturnValue>();
};

std::function<typename Engine::Object::SpawnReturnValue(
	const Engine::Object::SpawnParam&)>
	Engine::ThirdPersonCamera::PrototypeEdit()&
{
	return {};
};

void Engine::ThirdPersonCamera::SetUpTarget(const TargetInformation& TargetInformationParam)&
{
	_TargetInformation = TargetInformationParam;
};