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
// 슬롯은 AddColor 설정 아이콘은 AddColor = 0

namespace Engine
{
	class DLL_DECL UI
	{
	public :
		void Initialize(IDirect3DDevice9* const Device,
			const Vector2 Scale,
			const Vector2 Position,
			const std::filesystem::path& TexFullPath,
			const float UIDepthZ)&;
		void  Render(Engine::Renderer* const _Renderer)&;
		float GetUIDepthZ()const& {return UIDepthZ;};

		Vector2 Position{ 0.0f , 0.0f };
		Vector2 Scale   { 1.0f , 1.0f };

		static const inline Vector3 UISkillIconRed    { 255.f/ 255.f, 58.f  / 255.f,176.f/255.f       };
		static const inline Vector3 UISkillIconBlue   { 112.f / 255.f, 245.f / 255.f,255.f / 255.f };
		static const inline Vector3 UISkillIconGreen  { 163.f / 255.f, 255.f / 255.f,163.f / 255.f };
		float AlphaFactor = 1.f;
		bool bRender = true;
		int Flag = 1;
		Vector3 AddColor = UISkillIconRed;
		float CoolTimeHeight = 1.f;

		std::optional<Matrix> WorldUI{};
	private:
		uint32 ID = 0u;
		std::string TexFileName{};
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