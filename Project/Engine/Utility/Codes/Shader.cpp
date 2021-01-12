#include "Shader.h"

Engine::Shader Engine::Shader::Create(
	const std::wstring& ShaderFileName,	
	IDirect3DDevice9* const Device, 
	const std::vector<std::string>& VsConstantVariableNames, 
	const std::vector<std::string>& PsConstantVariableNames, 
	const std::vector<std::string>& SamplerNames)
{
	Shader _Shader;
	_Shader.CompileAndCreate(ShaderFileName, Device);
	_Shader.ConstantHandleInitialize(
		VsConstantVariableNames, PsConstantVariableNames);
	_Shader.ConstantHandleDescInitialize(SamplerNames);
	return _Shader;
}

void 
Engine::Shader::CompileAndCreate(
	const std::wstring& ShaderFileName, 
	IDirect3DDevice9* const Device)
{
	Shader _Shader;

	// ¹öÅØ½º
	{
		const std::wstring VsFileName = ShaderFileName + L"VS.hlsl";

		ID3DXBuffer* ShaderBuffer{nullptr};
		ID3DXBuffer* ErrorBuffer {nullptr};
		HRESULT Hr;
		ID3DXConstantTable* VsConstantTableTemp{ nullptr };
		Hr = D3DXCompileShaderFromFile(
			VsFileName.c_str(),
			0,
			0,
			"main",
			"vs_3_0",
			D3DXSHADER_DEBUG,
			&ShaderBuffer,
			&ErrorBuffer,
			&VsConstantTableTemp);

		if (ErrorBuffer)
		{
			throw std::exception(__FUNCTION__);
			//::MessageBoxA(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
			ErrorBuffer->Release();
		}

		_Shader.VsConstantTable = DX::MakeUnique(VsConstantTableTemp);

		if (FAILED(Hr))
			throw std::exception(__FUNCTION__);
		
		IDirect3DVertexShader9* VertexShaderTemp{ nullptr };
		Hr = Device->CreateVertexShader(
			(DWORD*)ShaderBuffer->GetBufferPointer(),
			&VertexShaderTemp);
		_Shader.VsShader = DX::MakeUnique(VertexShaderTemp);

		if (FAILED(Hr))
		{
			throw std::exception(__FUNCTION__);
		}
		ShaderBuffer->Release();
		_Shader.VsConstantTable->SetDefaults(Device);
	}

	// ÇÈ¼¿
	{
		ID3DXConstantTable* PsConstantTableTemp;

		ID3DXBuffer* ShaderBuffer{ nullptr };
		ID3DXBuffer* ErrorBuffer{ nullptr };
		HRESULT Hr;
		const std::wstring PsFileName = ShaderFileName + L"PS.hlsl";
		Hr = D3DXCompileShaderFromFile(
			PsFileName.c_str(),
			0,
			0,
			"main",
			"ps_3_0",
			D3DXSHADER_DEBUG,
			&ShaderBuffer,
			&ErrorBuffer,
			&PsConstantTableTemp);

		_Shader.PsConstantTable = DX::MakeUnique(PsConstantTableTemp);

		if (ErrorBuffer)
		{
			//::MessageBoxA(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
			throw std::exception(__FUNCTION__);
		}

		if (FAILED(Hr))
		{
			throw std::exception(__FUNCTION__);

		}
		IDirect3DPixelShader9* PixelShaderTemp{ nullptr };
		Hr = Device->CreatePixelShader(
			(DWORD*)ShaderBuffer->GetBufferPointer(),
			&PixelShaderTemp);
		_Shader.PsShader = DX::MakeUnique(PixelShaderTemp);

		if (FAILED(Hr))
		{
			throw std::exception(__FUNCTION__);
		}
		ShaderBuffer->Release();

		_Shader.PsConstantTable->SetDefaults(Device);
	};

}

void Engine::Shader::ConstantHandleInitialize(
	const std::vector<std::string>& VsConstantVariableNames, 
	const std::vector<std::string>& PsConstantVariableNames)
{
	for (const std::string& ConstantDataName : VsConstantVariableNames)
	{
		VsHandleMap[ConstantDataName] =
			VsConstantTable->GetConstantByName(0, ConstantDataName.c_str());
	}

	for (const std::string& ConstantDataName : PsConstantVariableNames)
	{
		PsHandleMap[ConstantDataName] =
			PsConstantTable->GetConstantByName(0, ConstantDataName.c_str());
	}
};


void Engine::Shader::ConstantHandleDescInitialize(
	const std::vector<std::string>& SamplerNames)
{
	std::unordered_map<std::string, D3DXCONSTANT_DESC> ConstantDescMap;
	
	uint32 Count = 0;
	for (auto& _ConstantTextureName : SamplerNames)
	{
		auto TexHandle = PsConstantTable->GetConstantByName(0, _ConstantTextureName.c_str());
		if (!TexHandle) { throw std::exception(__FUNCTION__); };
		D3DXCONSTANT_DESC Desc;
		PsConstantTable->GetConstantDesc(TexHandle, &Desc, &Count);
		ConstantDescMap[_ConstantTextureName] = Desc;
	};

	TextureDescMap = ConstantDescMap;
}

uint32 Engine::Shader::GetSamplerIndex(const std::string& SamplerName)
{
	return TextureDescMap.find(SamplerName)->second.RegisterIndex;
}

D3DXHANDLE Engine::Shader::GetVsConstantHandle(const std::string& HandleKey)
{
	return VsHandleMap.find(HandleKey)->second;
}

D3DXHANDLE Engine::Shader::GetPsConstantHandle(const std::string& HandleKey)
{
	return PsHandleMap.find(HandleKey)->second;
}
