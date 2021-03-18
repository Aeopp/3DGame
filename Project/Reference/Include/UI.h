#pragma once
#include "Vertexs.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include "FMath.hpp"
#include <filesystem>
#include "DllHelper.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"
#include <optional>
#include <set>
#include "ShaderFx.h"
#include "StringHelper.h"


// 월드 지정 해줘야함.
// 그리는 순서 포워드랑 같이하면 될듯 ?

namespace Engine
{
	class DLL_DECL UI
	{
	public :
		void Initialize(IDirect3DDevice9* const Device, 
						const Vector2& NDCLeftTopAnchor,
						const Vector2& NDCSize ,
						const std::filesystem::path& TexFullPath ,
						const float UIDepthZ                        )&;
		void Render(Engine::Renderer* const _Renderer)&;

		float GetUIDepthZ()const& {return UIDepthZ;};
	private:
		float UIDepthZ{ 0.0f };
		IDirect3DDevice9* Device{ nullptr };
		Engine::ShaderFx _ShaderFx{};
		uint32  Stride = 0u;
		IDirect3DTexture9* _Texture{ nullptr };
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		IDirect3DVertexBuffer9* VtxBuf{ nullptr };
		IDirect3DIndexBuffer9* IdxBuf{ nullptr };
	};
}