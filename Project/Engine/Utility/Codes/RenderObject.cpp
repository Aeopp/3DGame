#include "RenderObject.h"

void Engine::RenderObject::Initialize()&
{
	Super::Initialize();
}

void Engine::RenderObject::PrototypeInitialize(
	IDirect3DDevice9* const Device)&
{
	this->Device = Device;
};

void Engine::RenderObject::Event()&
{
	Super::Event();
};

void Engine::RenderObject::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);

};

void Engine::RenderObject::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};




