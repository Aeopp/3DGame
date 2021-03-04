#pragma once

#include "Component.h"
#include "RenderInterface.h"
#include "Vertexs.hpp"
#include <any>
#include "MaterialInformation.h"

namespace Engine
{
	struct DLL_DECL MeshElement
	{
		IDirect3DVertexBuffer9* VertexBuffer   { nullptr };
		IDirect3DIndexBuffer9*  IndexBuffer    { nullptr };
		DWORD  Stride        { 0u };
		uint32 FaceCount     { 0u };
		uint32 VtxCount      { 0u };
		uint32 PrimitiveCount{ 0u };
		uint32 VtxBufSize    { 0u };
		Engine::MaterialInformation MaterialInfo{};
	};

	class DLL_DECL Mesh : public Component 
	{
	public:
		using Super = Component;
	public:
		void Initialize(IDirect3DDevice9* const Device)&;
		virtual void Event(class Object* Owner) & override;

		virtual void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&;
		virtual void RenderShadowDepth(
			const Matrix& LightViewProjection)&;
		virtual void RenderDeferredAfter(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation,
			IDirect3DTexture9* const ShadowDepthMap,
			const Matrix& LightViewProjection,
			const float ShadowDepthMapSize,
			const float ShadowDepthBias,
			const Vector3& FogColor,
			const float FogDistance)&;

		virtual void RenderReady(Engine::Frustum& RefFrustum) &;
	public:
		static const inline Property TypeProperty = Property::Render;
		std::shared_ptr<std::vector<Vector3>> LocalVertexLocations;
	protected:
		std::wstring ResourceName{ };  
		IDirect3DDevice9* Device{ nullptr };

	};
}