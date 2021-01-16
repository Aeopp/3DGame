#include "DynamicCamera.h"
#include "Transform.h"
#include "Controller.h"
#include "imgui.h"

void Engine::DynamicCamera::Initialize(
	const float FovY,
	const float Near,
	const float Far,
	const float Aspect ,
	const float Speed,
	Controller* const _Control)&
{
	Super::Initialize(FovY , Near  ,Far , Aspect );
	this->_Control = _Control;
	this->Speed = Speed;
}

void Engine::DynamicCamera::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);

	auto _Transform = GetComponent<Transform>();

	if (bMouseFix)
	{
		const float Yaw =
			static_cast <float> (_Control->GetMouseMove(MouseMove::X)) * DeltaTime;
		const float Pitch =
			static_cast <float> (_Control->GetMouseMove(MouseMove::Y)) * DeltaTime;
		MouseFix();

		_Transform->Rotate({ 0,1,0 }, Yaw);
		_Transform->Rotate(_Transform->GetRight(), Pitch);
	}

	if (_Control->IsPressing(DIK_W))
	{
		_Transform->MoveForward(DeltaTime, Speed);
	}
	if (_Control->IsPressing(DIK_S))
	{
		_Transform->MoveForward(DeltaTime, -Speed);
	}
	if (_Control->IsPressing(DIK_D))
	{
		_Transform->MoveRight(DeltaTime, Speed);
	}
	if (_Control->IsPressing(DIK_A))
	{
		_Transform->MoveRight(DeltaTime, -Speed);
	}
};


void Engine::DynamicCamera::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
}
void Engine::DynamicCamera::Event()&
{
	if (_Control->IsDown(DIK_P))
	{
		MouseFixToggle();
	}
}
;

void Engine::DynamicCamera::PrototypeInitialize(
							IDirect3DDevice9* const Device ,
							const HWND Hwnd)&
{
	Super::PrototypeInitialize(Device);
	this->Hwnd = Hwnd;
};

void Engine::DynamicCamera::MouseFix()&
{
	D3DVIEWPORT9 ViewPort;
	Device->GetViewport(&ViewPort);
	POINT FixPosition{ static_cast<int32>(ViewPort.Width / 2u) ,
						static_cast<int32>(ViewPort.Height / 2u) };
	ClientToScreen(Hwnd, &FixPosition);
	SetCursorPos(FixPosition.x, FixPosition.y);
}
void Engine::DynamicCamera::MouseFixToggle()&
{
	bMouseFix = !bMouseFix;
}
;

