#include "Camera.h"
#include "Transform.h"
#include "imgui.h"
#include "FMath.hpp"

void Engine::Camera::Initialize(
	const float FovY,
	const float Near,
	const float Far,
	const float Aspect,
	const std::string& ClassIdentity
)&
{
	Super::Initialize();
	AddComponent<Transform>(ClassIdentity);
	this->FovY = FovY;
	this->Near = Near;
	this->Far = Far;
	this->Aspect = Aspect;
};


void Engine::Camera::Event()&
{
	Super::Event();

}

void Engine::Camera::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};


void Engine::Camera::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);

	const auto _Transform = GetComponent<Transform>();
	const Matrix View = FMath::Inverse(_Transform->UpdateWorld());
	Matrix Projection;
	D3DXMatrixPerspectiveFovLH(&Projection, FovY, Aspect, Near, Far);
	Device->SetTransform(D3DTS_VIEW, &View);
	Device->SetTransform(D3DTS_PROJECTION, &Projection);
};

void Engine::Camera::PrototypeInitialize(IDirect3DDevice9* const Device)&
{
	Super::PrototypeInitialize();

	this->Device = Device;
}
std::function<typename Engine::Object::SpawnReturnValue(
	const Engine::Object::SpawnParam&)> 
	Engine::Camera::PrototypeEdit()&
{
	return {};
};