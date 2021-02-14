#pragma once
#include "DllHelper.h"
#include "TypeAlias.h"
#include "SingletonInterface.h"
#include "RenderInterface.h"
#include <map>
#include <vector>
#include <d3d9.h>
#include "DxHelper.h"
#include "Frustum.h"
#include "Landscape.h"

namespace Engine
{
	class DLL_DECL Renderer : public SingletonInterface<Renderer>
	{
	public:
		void Initialize(
			const DX::SharedPtr<IDirect3DDevice9>& Device)&;
		void Render()&; 
		void Regist(RenderInterface* const Target);
		Landscape& RefLandscape()&;
	private:
		void RenderLandscape(Frustum& RefFrustum, const Matrix& View, const Matrix& Projection ,
			const Vector3& CameraLocation)&;
		void RenderDebugCollision()&;
		void RenderNoAlpha()&;
		void RenderEnviroment()&;
		void RenderUI()&;
	private:
		Landscape CurrentLandscape{};
		std::map<RenderInterface::Group, std::vector<std::reference_wrapper<RenderInterface>>>RenderObjects;
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
		Frustum _Frustum;
	};
};



