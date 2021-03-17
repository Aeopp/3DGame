#include <filesystem>
#include "StaticMesh.h"
#include "ResourceSystem.h"
#include <future>
#include <set>
#include <optional>
#include "UtilityGlobal.h"
#include "Renderer.h"
#include "imgui.h"

void Engine::StaticMesh::Initialize(
	const std::wstring& ResourceName)&
{
	
	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoStaticMesh =
		(*ResourceSys->GetAny<std::shared_ptr<Engine::StaticMesh>>(ResourceName));
	uint32 BackupID = ID; 
	this->operator=(*ProtoStaticMesh);
	ID = BackupID; 

	Super::Initialize(Device,_Group);

	ForwardShaderFx.Initialize(L"DefaultFx");
	DepthShadowFx.Initialize(L"ShadowDepthFx");
	DeferredDefaultFx.Initialize(L"DeferredAlbedoNormalVelocityDepthSpecularRimFx");
	DeferredDissolveFx.Initialize(L"DeferredDissolveAlbedoNormalVelocityDepthSpecularRimFx");
	VelocityFx.Initialize(L"VelocityFx");
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::StaticMesh::Render(Engine::Renderer* const _Renderer)&
{
	Super::Render(_Renderer);
}
void Engine::StaticMesh::RenderDeferredAlbedoNormalVelocityDepthSpecularRim(Engine::Renderer* const _Renderer)&
{
	Super::RenderDeferredAlbedoNormalVelocityDepthSpecularRim(_Renderer);
}
void Engine::StaticMesh::RenderShadowDepth(Engine::Renderer* const _Renderer)&
{
	Super::RenderShadowDepth(_Renderer);
}
void Engine::StaticMesh::RenderReady(Engine::Renderer* const _Renderer)&
{
	Super::RenderReady(_Renderer);
}

void Engine::StaticMesh::RenderVelocity(Engine::Renderer* const _Renderer)&
{
	Super::RenderVelocity(_Renderer);
}
