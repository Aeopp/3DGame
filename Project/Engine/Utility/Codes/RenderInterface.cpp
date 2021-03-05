#include "RenderInterface.h"
#include "Renderer.h"
#include "Transform.h"
#include "UtilityGlobal.h"
#include "imgui.h"

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
	if (bCullingOn)
	{
		bCurrentFrustumIn = RefFrustum.IsIn(GetCullingSphere());
	}
	else
	{
		bCurrentFrustumIn = true;
	}

	if (Engine::Global::bDebugMode && bCullingOn)
	{
		ImGui::TextColored(ImVec4{ 1.f,114.f / 255.f, 198.f / 255.f , 1.0f }, "Draw : %s",(DebugName).c_str());
	}

	return  bCurrentFrustumIn;
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

