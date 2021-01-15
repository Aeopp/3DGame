#include "ShaderManager.h"

void Engine::ShaderManager::Initialize(
	const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
};

void Engine::ShaderManager::Update(
	const Vector4& CameraLocation, const Vector4& LightLocation)
{
	
}

Engine::Shader& Engine::ShaderManager::EmplaceShader(
	const std::wstring& ShaderName,
	const std::wstring& ShaderFileName, 
	const std::vector<std::string>& VsConstantVariableNames, 
	const std::vector<std::string>& PsConstantVariableNames, 
	const std::vector<std::string>& SamplerNames)&
{
	return ShaderMap[ShaderName] = 
			  Shader::Create(ShaderFileName, 
			  Device.get(), VsConstantVariableNames,
			  PsConstantVariableNames, SamplerNames);
}

typename Engine::Shader& 
Engine::ShaderManager::RefShader(const std::wstring& ShaderName)
{
	return ShaderMap.find(ShaderName)->second;
}
