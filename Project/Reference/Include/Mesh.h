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

	class DLL_DECL Mesh : 
							public Component  ,
							public RenderInterface
	{
	public:
		using Super = Component;
	public:
		void Initialize(IDirect3DDevice9* const Device,
						const RenderInterface::Group _Group)&;
		virtual void Event(class Object* Owner) & override;

		// �⺻ ���̴� . (������)
		virtual void Render(Engine::Renderer* const _Renderer)& override;
		// ���� �н��� �ʿ��� �븻 �˺����� �Ӽ��� ����Ÿ�ٿ� ������. 
		virtual void RenderDeferredAlbedoNormalVelocityDepthSpecularRim(Engine::Renderer* const _Renderer)&override;
		// ������ �ʿ� �ʿ��� ������ ���������� ���������� ������. 
		virtual void RenderShadowDepth(Engine::Renderer* const _Renderer)& override;
		virtual void RenderReady(Engine::Renderer* const _Renderer)& override;
		virtual void RenderVelocity(Engine::Renderer* const _Renderer) & override;
	public:
		bool bDepthShadow{ true };
		static const inline Property TypeProperty = Property::Render;
		std::shared_ptr<std::vector<Vector3>> LocalVertexLocations;
	protected:
		Matrix PrevWorld = FMath::Identity();
		Matrix PrevView = FMath::Identity();
		Matrix OwnerWorld = FMath::Identity();
		IDirect3DVertexDeclaration9*                  VtxDecl{ nullptr };
		std::vector<MeshElement>                     MeshContainer{};
		Engine::Object* Owner{ nullptr };
		std::wstring ResourceName{ };  
		IDirect3DDevice9* Device{ nullptr };
	};
}