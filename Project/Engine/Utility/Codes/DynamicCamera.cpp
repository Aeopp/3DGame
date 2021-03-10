#include "DynamicCamera.h"
#include "Transform.h"
#include "Controller.h"
#include "imgui.h"
#include "FMath.hpp"
#include "UtilityGlobal.h"



void Engine::DynamicCamera::Initialize(
	const float FovY,
	const float Near,
	const float Far,
	const float Aspect ,
	const float Speed,
	Controller* const _Control)&
{
	Super::Initialize(FovY , Near  ,Far , Aspect  ,typeid(DynamicCamera).name());
	this->_Control    = _Control;
	this->Speed       = Speed;
}

void Engine::DynamicCamera::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
	
	auto _Transform = GetComponent<Transform>();

	if (bMouseFix)
	{
		const float Yaw =
			static_cast <float> (_Control->GetMouseMove(MouseMove::X)) ;
		const float Pitch =
			static_cast <float> (_Control->GetMouseMove(MouseMove::Y)) ;
		MouseFix();

		_Transform->RotateYaw(Yaw, DeltaTime);
		_Transform->RotatePitch(Pitch, DeltaTime);

		/*_Transform->RotateAxis({ 0,1,0 }, Yaw , DeltaTime );
		_Transform->RotateAxis(_Transform->GetRight(), Pitch, DeltaTime);*/
	}

	if (bMoveable)
	{
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
		if (_Control->IsPressing(DIK_Z))
		{
			_Transform->MoveUp(DeltaTime, +Speed);
		}
		if (_Control->IsPressing(DIK_X))
		{
			_Transform->MoveUp(DeltaTime, -Speed);
		}
	}
	
	if (bZoomable)
	{
		const float ZDelta = _Control->GetMouseMove(Engine::MouseMove::Z);
		if (false == FMath::AlmostEqual(ZDelta, 0.0f))
		{
			POINT Pt;
			GetCursorPos(&Pt);
			ScreenToClient(Engine::Global::Hwnd, &Pt);
			Vector3 Dir = { (float)Pt.x,(float)Pt.y,1.f };
			const Ray _Ray =
				FMath::GetRayScreenProjection
				(Dir, Device, Engine::Global::ClientSize.first, Engine::Global::ClientSize.second);

			FovY += (-ZDelta / 1080.f);
		}
	}
};


void Engine::DynamicCamera::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
}
void Engine::DynamicCamera::Event()&
{
	Super::Event();

	// ShowCursor(!bMouseFix);

	if (_Control->IsDown(DIK_F1))
	{
		MouseFixToggle();
	}
	if (_Control->IsDown(DIK_F2))
	{
		MoveableToggle();
	}
	if (_Control->IsDown(DIK_F3))
	{
		ZoomToggle();
	}
	if (_Control->IsDown(DIK_F4))
	{
		bMoveable = false;
		bZoomable = false;
		bMouseFix = false;
	}
	if (_Control->IsDown(DIK_F5))
	{
		bMoveable = true;
		bZoomable = true;
		bMouseFix = true;
	}
	
	if (Engine::Global::bDebugMode)
	{
		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::SliderFloat("Fov", &FovY,20.f,90.f);
			ImGui::SliderFloat("Speed", &Speed, 1.f, 1000.f);
			ImGui::SliderFloat("Far", &Far, 1.f, 100000.f);

			ImGui::InputFloat("Fov.", &FovY);
			ImGui::InputFloat("Speed.", &Speed);
			ImGui::InputFloat("Far.", &Far);
		}
	}
};

void Engine::DynamicCamera::PrototypeInitialize(
							IDirect3DDevice9* const Device ,
							const HWND Hwnd)&
{
	Super::PrototypeInitialize(Device);
	bCapturable = false;

	this->Hwnd = Hwnd;
}
std::shared_ptr<Engine::Object> Engine::DynamicCamera::GetCopyShared()&
{
	std::remove_pointer_t<decltype(this)> Clone = *this;
	return std::make_shared<Engine::DynamicCamera>(Clone);
}
std::optional<Engine::Object::SpawnReturnValue>  Engine::DynamicCamera::InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam)&
{
	return {};
};

void Engine::DynamicCamera::MouseFix()&
{
	D3DVIEWPORT9 ViewPort;
	Device->GetViewport(&ViewPort);
	POINT FixPosition{ static_cast<int32>(ViewPort.Width / 2u) ,
						static_cast<int32>(ViewPort.Height / 2u) };
	ClientToScreen(Hwnd, &FixPosition);
	SetCursorPos(FixPosition.x, FixPosition.y);
};

void Engine::DynamicCamera::MoveableToggle()&
{
	bMoveable = !bMoveable;
}

void Engine::DynamicCamera::ZoomToggle()&
{
	bZoomable = !bZoomable;
}
;
void Engine::DynamicCamera::MouseFixToggle()&
{
	bMouseFix = !bMouseFix;
};

