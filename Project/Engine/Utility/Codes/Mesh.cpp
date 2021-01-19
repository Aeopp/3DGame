#include "Mesh.h"



void Engine::Mesh::Initialize(IDirect3DDevice9* const Device, 
	const RenderInterface::Group _Group)&
{
	Super::Initialize();
	this->Device = Device;
	SetUpRenderingInformation(_Group);
}
