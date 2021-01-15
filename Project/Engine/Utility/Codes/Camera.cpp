#include "Camera.h"
#include "Transform.h"

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
	this->Aspect;
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
