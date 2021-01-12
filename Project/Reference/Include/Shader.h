#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "DxHelper.h"
#include <unordered_map>
#include <type_traits>

namespace Engine
{
	class  DLL_DECL Shader
	{
	private:
		Shader() = default;
	public:
		static Shader Create(const std::wstring& ShaderFileName,
			IDirect3DDevice9* const Device,
			const std::vector<std::string>& VsConstantVariableNames,
			const std::vector<std::string>& PsConstantVariableNames,
			const std::vector<std::string>& SamplerNames);
	private:
		void CompileAndCreate(
			const std::wstring& ShaderFileName,
			IDirect3DDevice9* const Device);

		void  ConstantHandleInitialize(
				const std::vector<std::string>& VsConstantVariableNames,
				const std::vector<std::string>& PsConstantVariableNames);

		void  ConstantHandleDescInitialize(
				const std::vector<std::string>& SamplerNames);
	public:
		uint32  GetSamplerIndex(const std::string& SamplerName);
		D3DXHANDLE GetVsConstantHandle(const std::string& HandleKey);
		D3DXHANDLE GetPsConstantHandle(const std::string& HandleKey);
	public:
		template<typename Type>
		void SetVSCostantData(const DX::SharedPtr<IDirect3DDevice9>& Device,
			const std::string& ConstantHandleMapKey,const Type& Data,
			const uint32 Num = 1);
		template<typename Type>
		void SetPSCostantData(const DX::SharedPtr<IDirect3DDevice9>& Device,
			const std::string& ConstantHandleMapKey, const Type& Data,
			const uint32 Num = 1);
	private:
		template<typename Type>
		void SetCostantDataImplementation
			(   DX::UniquePtr<ID3DXConstantTable>& ConstantTable,
				std::unordered_map<std::string,D3DXHANDLE>& HandleMap,
				DX::SharedPtr<IDirect3DDevice9>& Device,
			const std::string& ConstantHandleMapKey, const Type& Data,
			const uint32 Num = 1);
	private:
		DX::UniquePtr<IDirect3DPixelShader9> PsShader{ nullptr };
		DX::UniquePtr<ID3DXConstantTable> PsConstantTable{ nullptr };
		DX::UniquePtr<IDirect3DVertexShader9> VsShader{ nullptr };
		DX::UniquePtr<ID3DXConstantTable> VsConstantTable{ nullptr };
		std::unordered_map<std::string, D3DXHANDLE> VsHandleMap;
		std::unordered_map<std::string, D3DXHANDLE> PsHandleMap;
		std::unordered_map<std::string, D3DXCONSTANT_DESC>TextureDescMap;
	};
};


template<typename Type>
inline void Engine::Shader::SetVSCostantData(const DX::SharedPtr<IDirect3DDevice9>& Device, const std::string& ConstantHandleMapKey, const Type& Data, const uint32 Num)
{
	SetCostantDataImplementation(VsConstantTable,
		VsHandleMap,Device, ConstantHandleMapKey, Data, Num);
}
template<typename Type>
inline void Engine::Shader::SetPSCostantData(const DX::SharedPtr<IDirect3DDevice9>& Device, const std::string& ConstantHandleMapKey, const Type& Data, const uint32 Num)
{
	SetCostantDataImplementation(PsConstantTable,
		PsHandleMap,Device, ConstantHandleMapKey, Data, Num);
}

template<typename Type>
inline void Engine::Shader::SetCostantDataImplementation(
	DX::UniquePtr<ID3DXConstantTable>& ConstantTable,
	std::unordered_map<std::string, D3DXHANDLE>& HandleMap, 
	 DX::SharedPtr<IDirect3DDevice9>& Device, const std::string& ConstantHandleMapKey, const Type& Data, const uint32 Num)
{
	const uint32 DataSize = sizeof(std::decay_t<Type>) * Num;
	
	if (FAILED(ConstantTable->SetValue(Device, HandleMap[ConstantHandleMapKey],
		reinterpret_cast<const void*>(&Data), DataSize)))
		throw std::exception(__FUNCTION__);
}
