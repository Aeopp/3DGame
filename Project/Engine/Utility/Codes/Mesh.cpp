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
};

void Engine::Mesh::RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Frustum& RefFrustum, const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&
{
}
void Engine::Mesh::RenderShadowDepth(const Matrix& LightViewProjection)&
{
	
}
void Engine::Mesh::RenderDeferredAfter(Engine::Frustum& RefFrustum, const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation, IDirect3DTexture9* const ShadowDepthMap, const Matrix& LightViewProjection, const float ShadowDepthMapSize, const float ShadowDepthBias, const Vector3& FogColor, const float FogDistance)&
{

};

void Engine::Mesh::RenderReady(Engine::Frustum& RefFrustum)&
{

};
