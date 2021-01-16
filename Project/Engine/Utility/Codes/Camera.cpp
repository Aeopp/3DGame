#include "Camera.h"
#include "Transform.h"
#include "imgui.h"
#include "FMath.hpp"

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

Vector3 gForward;

Vector3 gRight;
Vector3 gUp;
void Engine::Camera::Event()&
{
	auto _Transform = GetComponent<Transform>();
	const auto& From = _Transform->UpdateWorld();

	gForward = Vector3({ From._31,From._32,From._33 });;
	gRight = Vector3({ From._11,From._12,From._13 });;
	gUp = Vector3({ From._21,From._22,From._23 });;

	ImGui::Begin("TestTransform");
	ImGui::Text(
		"gForward\n X : %f \n Y : %f \n Z : %f", gForward.x, gForward.y, gForward.z);
	ImGui::Text(
		"gRight\n X : %f \n Y : %f \n Z : %f", gRight.x, gRight.y, gRight.z);
	ImGui::Text(
		"gUp\n X : %f \n Y : %f \n Z : %f", gUp.x, gUp.y, gUp.z);
	ImGui::End();

	ImGui::Begin("CameraInfo");

	const Vector3& Scale = _Transform->GetScale();
	const Vector3& Location = _Transform->GetLocation();
	const Vector3& Forward = _Transform->GetForward();
	const Vector3& Right = _Transform->GetRight();
	const Vector3& Up = _Transform->GetUp();
	const Vector3& Rotation = _Transform->GetRotation();

	ImGui::Text(
		"Scale\n X : %f \n Y : %f \n Z : %f", Scale.x, Scale.y, Scale.z);
	ImGui::Text(
		"Location\n X : %f \n Y : %f \n Z : %		f", Location.x, Location.y, Location.z);
	ImGui::Text("Rotation\n X : %f \n Y : %f \n Z : %f",
		Rotation.x, Rotation.y, Rotation.z);
	ImGui::Text(
		"Forward\n X : %f \n Y : %f \n Z : %f", Forward.x, Forward.y, Forward.z);
	ImGui::Text(
		"Right\n X : %f \n Y : %f \n Z : %f", Right.x, Right.y, Right.z);
	ImGui::Text(
		"Up\n X : %f \n Y : %f \n Z : %f", Up.x, Up.y, Up.z);
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