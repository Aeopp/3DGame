#include "StaticMesh.h"
#include "ResourceSystem.h"

void Engine::StaticMesh::Initialize(
	IDirect3DDevice9* const Device, 
	const RenderInterface::Group _Group,
	const std::wstring& FilePath,
	const std::wstring& FileName)&
{
	Super::Initialize(Device, _Group);

	auto& ResourceSys = ResourceSystem::Instance;

}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}
