#include "ShaderManager.h"

void Engine::ShaderManager::Initialize(
	const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;

};

void Engine::ShaderManager::Update(const Vector4& CameraLocation, const Vector4& LightLocation)
{

}

typename Engine::Shader& 
Engine::ShaderManager::RefShader(const std::wstring& ShaderName)
{
	return ShaderMap.find(ShaderName)->second;
}
