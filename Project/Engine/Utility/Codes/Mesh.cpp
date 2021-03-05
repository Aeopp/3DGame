#include "Mesh.h"
#include "Renderer.h"

void Engine::Mesh::Initialize(
	IDirect3DDevice9* const Device,
	const RenderInterface::Group _Group)&
{
	Super::Initialize();
	this->Device = Device;
	RenderInterface::SetUpRenderingInformation(_Group);
};

void Engine::Mesh::Event(Object* Owner)&
{
	Super::Event(Owner);
	RenderInterface::Regist();
	this->Owner = Owner;
}
void Engine::Mesh::Render(Engine::Renderer* const _Renderer)&
{
};
void Engine::Mesh::RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Renderer* const _Renderer)&
{
}; 
void Engine::Mesh::RenderShadowDepth(Engine::Renderer* const _Renderer)&
{
}; 
void Engine::Mesh::RenderReady(Engine::Renderer* const _Renderer)&
{
};