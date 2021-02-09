#pragma once
#include <filesystem>
#include "TypeAlias.h"
#include "DllHelper.H"
#include "DxHelper.h"
#include <unordered_map>
#include <type_traits>

namespace Engine
{
	class  DLL_DECL ShaderFx
	{
	public:
		void Initialize(const std::wstring& ResourceName)&;
		// 쉐이더 컴파일 이후 리소스 관리자에게 넘깁니다.
		static ShaderFx*
			Load(IDirect3DDevice9* const Device, const std::filesystem::path Path,
				const std::wstring& ResourceName);
		ID3DXEffect* GetHandle() const& { return EffectFx.get(); };
	private:
		IDirect3DDevice9* Device{ nullptr };
		DX::SharedPtr<ID3DXEffect> EffectFx{ nullptr };
	};
};