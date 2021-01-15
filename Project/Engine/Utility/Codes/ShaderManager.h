#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include "DxHelper.h"
#include "Shader.h"
#include <unordered_map>
namespace Engine
{
	class DLL_DECL ShaderManager : public SingletonInterface<ShaderManager>
	{
	public:
		void Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&;
		void Update(const Vector4& CameraLocation, const Vector4& LightLocation);
		Engine::Shader& AddShader(
			const std::wstring& ShaderName,
			const std::wstring& ShaderFileName,
			const std::vector<std::string>& VsConstantVariableNames,
			const std::vector<std::string>& PsConstantVariableNames,
			const std::vector<std::string>& SamplerNames)&;
		Shader& RefShader(const std::wstring& ShaderName);
	private:
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
		std::unordered_map<std::wstring, Shader> ShaderMap;
	};
};



