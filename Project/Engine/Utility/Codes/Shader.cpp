#include "Shader.h"

void Engine::Shader::Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
}
