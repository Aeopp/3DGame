#include "RenderInterface.h"
#include "Renderer.h"
#include "Transform.h"

bool FrustumCullingCheck(const Engine::Frustum& RefFrustum)&
{

}

Sphere Engine::RenderInterface::GetCullingSphere() const&
{
	Sphere WorldSphere = CullingLocalSphere;

	WorldSphere.Center = 
		FMath::Mul(WorldSphere.Center, RenderObjectTransform->UpdateWorld());
	const Vector3 Scale = RenderObjectTransform->GetScale();
	WorldSphere.Radius *= ((Scale.x + Scale.y + Scale.z) / 3.f );

	return WorldSphere;
}

bool Engine::RenderInterface::FrustumInCheck(Engine::Frustum& RefFrustum)&
{
	return bCurrentFrustumIn = RefFrustum.IsIn(GetCullingSphere());
}

void Engine::RenderInterface::SetUpCullingInformation(const Sphere				CullingLocalSphere,
		class Transform* const RenderObjectTransform)&

{
	this->RenderObjectTransform = RenderObjectTransform;
	this->CullingLocalSphere = CullingLocalSphere;
}

void Engine::RenderInterface::SetUpRenderingInformation(const Group _Group)
{
	this->_Group = _Group;

}

void Engine::RenderInterface::Regist()
{
	Renderer::Instance->Regist(this);
}

void Engine::RenderInterface::RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Renderer* const _Renderer)&
{
}

void Engine::RenderInterface::RenderShadowDepth(Engine::Renderer* const _Renderer)&
{
}

void Engine::RenderInterface::RenderReady(Engine::Renderer* const _Renderer)&
{
}

