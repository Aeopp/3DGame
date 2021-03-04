#pragma once
#include "Object.h"
#include "RenderInterface.h"
#include "MathStruct.h"
#include "Frustum.h"

// �ش� Ŭ������ ������ ���õ� ������Ʈ�� ���� ���� ���� �������� �����ϴ� ������Ʈ.
namespace Engine
{
	class DLL_DECL RenderObject : public Object , public RenderInterface
	{
	public:
		using Super = Object; 
	public:
		void Initialize()&;
		void PrototypeInitialize(IDirect3DDevice9* const Device ,
							const RenderInterface::Group _Group )&;
	public:
		virtual void Event()&;
		virtual void Render(const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation)& abstract;

		virtual void RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(Engine::Frustum& RefFrustum,
			const Matrix& View, const Matrix& Projection, const Vector4& CameraLocation)&;

		virtual void RenderShadowDepth(
			const Matrix& LightViewProjection)&;

		virtual void LateUpdate(const float DeltaTime) & override;
		virtual void Update(const float DeltaTime) & override;
	protected:
		IDirect3DDevice9* Device{ nullptr };
	};
};



