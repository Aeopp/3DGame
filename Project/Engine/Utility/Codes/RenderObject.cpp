#include "RenderObject.h"

void Engine::RenderObject::Initialize()&
{
	Super::Initialize();
}

void Engine::RenderObject::PrototypeInitialize(
	IDirect3DDevice9* const Device,
	const RenderInterface::Group _Group)&
{
	RenderInterface::SetUpRenderingInformation(_Group);
	this->Device = Device;  
}
void Engine::RenderObject::Event()&
{
	Super::Event();
};

void Engine::RenderObject::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
	RenderInterface::Regist();
}
void Engine::RenderObject::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}
;

