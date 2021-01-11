#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include "DxHelper.h"
#include <unordered_map>

namespace Engine
{
	class DLL_DECL Shader : public SingletonInterface<Shader>
	{
	public:
		struct Information
		{
			DX::UniquePtr<IDirect3DPixelShader9> PsShader{ nullptr };
			DX::UniquePtr<ID3DXConstantTable> PsConstantTable{ nullptr };
			DX::UniquePtr<IDirect3DVertexShader9> VsShader{ nullptr };
			DX::UniquePtr<ID3DXConstantTable> VsConstantTable{ nullptr };
			std::unordered_map<std::string, D3DXHANDLE> VsHandleMap;
			std::unordered_map<std::string, D3DXHANDLE> PsHandleMap;
			std::unordered_map<std::string, D3DXCONSTANT_DESC>TextureDescMap;
		};
	public:
		void Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&;
	public:
		std::unordered_map<std::wstring, Information> ShaderInfoMap;
	private:
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
	};
};



