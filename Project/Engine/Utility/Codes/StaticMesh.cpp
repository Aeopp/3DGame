#include "StaticMesh.h"

void Engine::StaticMesh::Initialize(IDirect3DDevice9* const Device, const RenderInterface::Group _Group)&
{
	Super::Initialize(Device, _Group);
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}
