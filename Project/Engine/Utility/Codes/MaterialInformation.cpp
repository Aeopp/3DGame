#include "MaterialInformation.h"
#include <istream>
#include <fstream>
#include <d3dx9.h>
#include "ResourceSystem.h"
#include <ostream>
#include "UtilityGlobal.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include "StringHelper.h"

void Engine::MaterialInformation::Load
(
	IDirect3DDevice9* const Device,
	const std::filesystem::path& MatFilePath,
	const std::filesystem::path& MatFileName,
	const std::wstring& TexFileExtenstion)&
{
	std::wifstream MatStream{ MatFilePath / MatFileName };
	std::wstring TexLine{};
	std::wstring TexKey{};
	std::wstring TexFileName{};

	Name = ToA(MatFileName);

	while (MatStream)
	{
		std::getline(MatStream, TexLine);
		if (TexLine.empty())continue;

		std::erase_if(TexLine,[](const char DeleteSpace) 
			{
				return DeleteSpace == L' ';
			});

		const size_t SeparatorOffset = TexLine.find('=');
		if (SeparatorOffset == std::wstring::npos)break;

		TexKey.assign(std::begin(TexLine), std::begin(TexLine) + SeparatorOffset);
		TexFileName.assign(std::begin(TexLine) + (SeparatorOffset + 1), std::end(TexLine));

		TexFileName += TexFileExtenstion;

		auto& ResourceSys = ResourceSystem::Instance;

		IDirect3DTexture9* _Texture{ nullptr };
		_Texture = ResourceSys->Get<IDirect3DTexture9>(TexFileName);

		if (_Texture == nullptr)
		{
			D3DXCreateTextureFromFile(Device, (MatFilePath / L"Texture" / TexFileName).c_str(), &_Texture);
			if (_Texture)
			{
				_Texture = ResourceSys->Insert<IDirect3DTexture9>(TexFileName, _Texture);
			}
		}

		MaterialTexture MtTex{}; 
		MtTex.Texture = _Texture; 
		MaterialTextureMap.insert({ ToA(TexKey),std::move(MtTex) });

		TexLine.clear();
		TexKey.clear();
		TexFileName.clear();
	}

	MaterialTextureMap["DefaultDiffuse"].Texture  = DefaultTextures["DiffuseMap"];
	MaterialTextureMap["DefaultNormal"].Texture   = DefaultTextures["NormalMap"];
	MaterialTextureMap["DefaultCavity"].Texture   = DefaultTextures["CavityMap"];

	MaterialTextureMap["DefaultDetailDiffuse"].Texture = DefaultTextures["DetailDiffuseMap"];
	MaterialTextureMap["DefaultDetailNormal"].Texture = DefaultTextures["DetailNormalMap"];
	MaterialTextureMap["DefaultEmissive"].Texture = DefaultTextures["DetailDiffuseMap"];

	PropPath = MatFilePath / (MatFileName.stem().wstring() + L".props");
	PropsLoad(PropPath);
}

void Engine::MaterialInformation::PropsLoad(const std::filesystem::path& PropsFilePath)&
{
	std::ifstream PropsStream{ PropsFilePath };

	if (PropsStream.is_open())
	{
		using namespace rapidjson;
		if (!PropsStream.is_open()) return;

		IStreamWrapper Isw(PropsStream);
		Document _Document;
		_Document.ParseStream(Isw);

		if (_Document.HasParseError())
		{
			//MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
			return;
		}
		
		const auto& RegisterKeyMap = _Document["RegisterKey"];
		for (auto TexRegisterNameIter = RegisterKeyMap.MemberBegin();
			TexRegisterNameIter != RegisterKeyMap.MemberEnd(); ++TexRegisterNameIter)
		{
			BindingMapping(TexRegisterNameIter->name.GetString(), TexRegisterNameIter->value.GetString());
		}

		Contract = _Document["Contract"].GetInt();
		RimInnerWidth = _Document["RimInnerWidth"].GetFloat();
		RimOuterWidth = _Document["RimOuterWidth"].GetFloat();
		Power = _Document["Power"].GetFloat();
		SpecularIntencity = _Document["SpecularIntencity"].GetFloat();
		DetailScale = _Document["DetailScale"].GetFloat();
		CavityCoefficient = _Document["CavityCoefficient"].GetFloat();
		DetailDiffuseIntensity = _Document["DetailDiffuseIntensity"].GetFloat();
		DetailNormalIntensity = _Document["DetailNormalIntensity"].GetFloat();

		if (_Document.HasMember("AlphaAddtive"))
		{
			AlphaAddtive = _Document["AlphaAddtive"].GetFloat();
		}

		if (_Document.HasMember("bForwardRender"))
		{
			bForwardRender = _Document["bForwardRender"].GetBool();
		}

		const auto&     RimAmtColorArr = _Document["RimAmtColor"].GetArray();
		RimAmtColor.x = RimAmtColorArr[0].GetFloat();
		RimAmtColor.y = RimAmtColorArr[1].GetFloat();
		RimAmtColor.z = RimAmtColorArr[2].GetFloat();
		RimAmtColor.w = RimAmtColorArr[3].GetFloat();

		const auto&      AmbientColorArr =  _Document["AmbientColor"].GetArray();
		AmbientColor.x = AmbientColorArr [0].GetFloat();
		AmbientColor.y = AmbientColorArr [1].GetFloat();
		AmbientColor.z = AmbientColorArr [2].GetFloat();
		AmbientColor.w = AmbientColorArr [3].GetFloat();
	}
}

IDirect3DTexture9* Engine::MaterialInformation::GetTexture(const std::string& MatTexKey) const&
{
	if (auto iter = MaterialTextureMap.find(MatTexKey);
		iter != std::end(MaterialTextureMap))
	{
		return iter->second.Texture;
	}
	else
	{
		return nullptr;  
	}
}

IDirect3DTexture9* Engine::MaterialInformation::GetTextureFromRegisterKey(const std::string& RegisterTexKey) const&
{
	for (const auto& MtTex : MaterialTextureMap)
	{
		if (MtTex.second.RegisterBindKey == RegisterTexKey)
			return MtTex.second.Texture;
	};

	return nullptr; 
}

void Engine::MaterialInformation::PropSave(std::filesystem::path PropsFilePath)&
{
	if (PropsFilePath.empty())
	{
		PropsFilePath = PropPath;
	}

	std::ofstream  PropStream{ PropsFilePath };

	if (PropStream.is_open())
	{
		using namespace rapidjson;

		StringBuffer StrBuf;

		PrettyWriter<StringBuffer> Writer(StrBuf);

		Writer.StartObject();
		{
			Writer.Key("RegisterKey");
			Writer.StartObject();
			for (const auto&  [TexKey,MtTexInfo] :MaterialTextureMap)
			{
				if (!MtTexInfo.RegisterBindKey.empty())
				{
					Writer.Key(TexKey.c_str());
					Writer.String(MtTexInfo.RegisterBindKey.c_str());
				}
			}
			Writer.EndObject();

			Writer.Key("Contract");
			Writer.Int(Contract);

			Writer.Key("RimInnerWidth");
			Writer.Double(RimInnerWidth);

			Writer.Key("RimOuterWidth");
			Writer.Double(RimOuterWidth);

			Writer.Key("Power");
			Writer.Double(Power);

			Writer.Key("SpecularIntencity");
			Writer.Double(SpecularIntencity);

			Writer.Key("DetailScale");
			Writer.Double(DetailScale);

			Writer.Key("CavityCoefficient");
			Writer.Double(CavityCoefficient);

			Writer.Key("DetailDiffuseIntensity");
			Writer.Double(DetailDiffuseIntensity);

			Writer.Key("DetailNormalIntensity");
			Writer.Double(DetailNormalIntensity);

			Writer.Key("RimAmtColor");
			Writer.StartArray();
			{
				Writer.Double(RimAmtColor.x);
				Writer.Double(RimAmtColor.y);
				Writer.Double(RimAmtColor.z);
				Writer.Double(RimAmtColor.w);
			}
			Writer.EndArray();

			Writer.Key("AmbientColor");
			Writer.StartArray();
			{
				Writer.Double(AmbientColor.x);
				Writer.Double(AmbientColor.y);
				Writer.Double(AmbientColor.z);
				Writer.Double(AmbientColor.w);
			}
			Writer.EndArray();

			Writer.Key("AlphaAddtive");
			Writer.Double(AlphaAddtive); 

			Writer.Key("bForwardRender");
			Writer.Bool(bForwardRender);
		}
		Writer.EndObject();

		PropStream << StrBuf.GetString();
	}
}

void Engine::MaterialInformation::BindingTexture(ID3DXEffect*  _Fx)const &
{
	for (const auto& [FxBindKey, _MaterialTex] : MaterialTextureMap)
	{
		if (!_MaterialTex.RegisterBindKey.empty())
		{
			_Fx->SetTexture(_MaterialTex.RegisterBindKey.c_str(), _MaterialTex.Texture);
		}
	}
}

void Engine::MaterialInformation::BindingMapping(
							const std::string& TexKey, const std::string& RegisterKey)&
{
	if (auto MtTexIter = MaterialTextureMap.find(TexKey);
		MtTexIter != std::end(MaterialTextureMap))
	{
		MtTexIter->second.RegisterBindKey = RegisterKey;
	}
}

void Engine::MaterialInformation::SetUpDefaultTexture()
{
	auto& _ResourceSys=ResourceSystem::Instance;
		
	IDirect3DTexture9* DefaultDiffuse =_ResourceSys->Get<IDirect3DTexture9>(L"Texture_DefaultDiffuse");
	IDirect3DTexture9* DefaultNormal = _ResourceSys->Get<IDirect3DTexture9>(L"Texture_DefaultNormal");
	IDirect3DTexture9* DefaultCavity = _ResourceSys->Get<IDirect3DTexture9>(L"Texture_DefaultCavity");

	DefaultTextures["EmissiveMap"]  = DefaultTextures ["DetailDiffuseMap"] = 	DefaultTextures["DiffuseMap"] = DefaultDiffuse;
	DefaultTextures["DetailNormalMap"] = DefaultTextures["NormalMap"] = DefaultNormal;
	DefaultTextures["CavityMap"] = DefaultCavity;
}
