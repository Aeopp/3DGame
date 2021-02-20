#include "ShaderFx.h"
#include "ResourceSystem.h"
#include <memory>

void Engine::ShaderFx::Initialize(const std::wstring& ResourceName)&
{
	auto& ResourceSys = Engine::ResourceSystem::Instance;
	auto ProtoShaderFx = *ResourceSys->GetAny<std::shared_ptr<Engine::ShaderFx>>(ResourceName);
	this->operator=(*ProtoShaderFx);
}

Engine::ShaderFx* Engine::ShaderFx::Load(
	IDirect3DDevice9* const Device,
	const std::filesystem::path Path,
	const std::wstring& ResourceName)
{
	std::shared_ptr<ShaderFx> _ShaderFx = std::make_shared<ShaderFx >();
	_ShaderFx->Device = Device;
	// Effect 성공 ErrMsg 실패 -> 정상적 컴파일
	// Effect 실패 ErrMsg 실패 -> 컴파일 실패
	// Effect 성공 ErrMsg 성공 -> 컴파일 성공 그러나 경고상태.
	ID3DXEffect* EffectInstance{};
	ID3DXBuffer* ErrMsgBufInstance{};

	if (FAILED(D3DXCreateEffectFromFile(Device, Path.c_str(),
		NULL, // 매크로 파일
		NULL, // 포함파일
		D3DXSHADER_DEBUG, // 디버깅을 제공하겠다는 플래그
		NULL,
		&EffectInstance,
		&ErrMsgBufInstance)))
	{
		MessageBoxA(NULL, (char*)ErrMsgBufInstance->GetBufferPointer(), "ShaderError", MB_OK);
		ErrMsgBufInstance->Release();
	}
	else if (nullptr != ErrMsgBufInstance)
	{
		MessageBoxA(NULL, (char*)ErrMsgBufInstance->GetBufferPointer(), "ShaderWarning", MB_OK);
		ErrMsgBufInstance->Release();
	}

	_ShaderFx->EffectFx = DX::MakeShared<ID3DXEffect>(EffectInstance);
	auto& ResourceSys = Engine::ResourceSystem::Instance;
	ResourceSys->InsertAny<std::shared_ptr<Engine::ShaderFx>>(ResourceName, _ShaderFx);

	return _ShaderFx.get();
}