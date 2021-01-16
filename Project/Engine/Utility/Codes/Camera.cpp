#include "Camera.h"
#include "Transform.h"
#include "imgui.h"

void Engine::Camera::Initialize(
	const float FovY,
	const float Near,
	const float Far,
	const float Aspect
)&
{
	Super::Initialize();
	AddComponent<Transform>();
	this->FovY = FovY;
	this->Near = Near;
	this->Far = Far;
	this->Aspect = Aspect;
}

void Engine::Camera::Event()&
{
	ImGui::Begin("CameraInfo");
	auto _Transform = GetComponent<Transform>();
	const Vector3& Location =_Transform->GetLocation();
	const Vector3& Rotation =_Transform->GetRotation();
	ImGui::Text(
		"Location\n X : %f \n Y : %f \n Z : %f",Location.x,Location.y,Location.z);
	ImGui::Text("Rotation\n X : %f \n Y : %f \n Z : %f", 
		Rotation.x, Rotation.y, Rotation.z);
	ImGui::End();
}

void Engine::Camera::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};


void Engine::Camera::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);

	const auto _Transform = GetComponent<Transform>();
	_Transform->UpdateWorld();
	const Vector3 Location = _Transform->GetLocation();
	const Vector3 Forward = _Transform->GetForward();
	const Vector3 Up = _Transform->GetUp();
	const Vector3 At = Location + Forward;
	Matrix View, Projection;
	D3DXMatrixLookAtLH(&View, &Location, &At, &Up);
	D3DXMatrixPerspectiveFovLH(&Projection, FovY, Aspect, Near, Far);
	Device->SetTransform(D3DTS_VIEW, &View);
	Device->SetTransform(D3DTS_PROJECTION, &Projection);
};

void Engine::Camera::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();
	
	this->Device = Device; 
};
