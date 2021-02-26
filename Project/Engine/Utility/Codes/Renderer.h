#pragma once
#include "DllHelper.h"
#include "TypeAlias.h"
#include <filesystem>
#include "SingletonInterface.h"
#include "RenderInterface.h"
#include <map>
#include <vector>
#include <d3d9.h>
#include "DxHelper.h"
#include "Frustum.h"
#include "Landscape.h"
#include "Sky.h"

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
		Vector4 LightDirection{ 0.707f, -0.707f  ,0.f  ,0.f};
		Vector4 LightColor{ 1,1,1,1 }; 
		void SkyInitialize(const std::filesystem::path& FullPath)&;
	private:
		void RenderLandscape(Frustum& RefFrustum, const Matrix& View, const Matrix& Projection ,
			                                       const Vector4 & CameraLocation)&;
		void RenderDebugCollision(const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation)&;
		void RenderNoAlpha(const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation)&;
		void RenderEnviroment(const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation)&;
		void RenderUI(const Matrix& View, const Matrix& Projection,
			const Vector4& CameraLocation)&;

		
	private:
		Sky _Sky{};
		Landscape CurrentLandscape{};
		std::map<RenderInterface::Group, std::vector<std::reference_wrapper<RenderInterface>>>RenderObjects;
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
		Frustum _Frustum;
	};
};



