#include "ShaderManager.h"
#include "UtilityGlobal.h"

void Engine::ShaderManager::Initialize(
	const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
	
	EmplaceShader(
		L"Skeleton", Global::ResourcePath  / L"Shader" / L"Skeleton",
		{ "World","ViewProjection","FinalMatrix" },
		{},
		{ "DiffuseSampler" });
};

void Engine::ShaderManager::Update(
	const Vector3& CameraLocation,const Vector3& LightLocation)&
{
	
}

Engine::Shader& Engine::ShaderManager::EmplaceShader(
	const std::wstring& ShaderName,
	const std::wstring& ShaderFileName, 
	const std::vector<std::string>& VsConstantVariableNames, 
	const std::vector<std::string>& PsConstantVariableNames, 
	const std::vector<std::string>& SamplerNames)&
{
	return ShaderMap.insert({ ShaderName ,
		Shader::Create(ShaderFileName,
			Device.get(), VsConstantVariableNames,
			PsConstantVariableNames, SamplerNames) }).first->second;
}

typename Engine::Shader& 
Engine::ShaderManager::RefShader(const std::wstring& ShaderName)
{
	return ShaderMap.find(ShaderName)->second;
}
