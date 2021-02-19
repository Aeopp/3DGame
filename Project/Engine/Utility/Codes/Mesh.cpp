#include "Mesh.h"
#include "Renderer.h"

void Engine::Mesh::Initialize(IDirect3DDevice9* const Device)&
{
	Super::Initialize();
	this->Device = Device;
}

void Engine::Mesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::Mesh::Render(const Matrix& World,
	const Matrix& View,
	const Matrix& Projection,
	const Vector4& CameraLocation4D)&
{

}
