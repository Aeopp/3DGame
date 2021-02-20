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
	// Effect ���� ErrMsg ���� -> ������ ������
	// Effect ���� ErrMsg ���� -> ������ ����
	// Effect ���� ErrMsg ���� -> ������ ���� �׷��� ������.
	ID3DXEffect* EffectInstance{};
	ID3DXBuffer* ErrMsgBufInstance{};

	if (FAILED(D3DXCreateEffectFromFile(Device, Path.c_str(),
		NULL, // ��ũ�� ����
		NULL, // ��������
		D3DXSHADER_DEBUG, // ������� �����ϰڴٴ� �÷���
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