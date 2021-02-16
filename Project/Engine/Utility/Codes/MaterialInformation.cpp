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

	while (MatStream)
	{
		std::getline(MatStream, TexLine);
		if (TexLine.empty())continue;

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
			_Texture = ResourceSys->Insert<IDirect3DTexture9>(TexFileName, _Texture);
		}

		TextureMap.insert({ TexKey,_Texture });

		TexLine.clear();
		TexKey.clear();
		TexFileName.clear();
	}


	if (TextureMap.contains(L"Cavity"))
	{
		bCavity = true;
	}

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

		Contract = _Document["Contract"].GetInt();

		RimInnerWidth = _Document["RimInnerWidth"].GetFloat();

		RimOuterWidth = _Document["RimOuterWidth"].GetFloat();

		Power = _Document["Power"].GetFloat();

		SpecularIntencity = _Document["SpecularIntencity"].GetFloat();

		const auto& RimAmtColorArr = _Document["RimAmtColor"].GetArray();
		RimAmtColor.x = RimAmtColorArr[0].GetFloat();
		RimAmtColor.y = RimAmtColorArr[1].GetFloat();
		RimAmtColor.z = RimAmtColorArr[2].GetFloat();
		RimAmtColor.w = RimAmtColorArr[3].GetFloat();

		const auto& AmbientColorArr =  _Document["AmbientColor"].GetArray();
		AmbientColor.x = AmbientColorArr [0].GetFloat();
		AmbientColor.y = AmbientColorArr [1].GetFloat();
		AmbientColor.z = AmbientColorArr [2].GetFloat();
		AmbientColor.w = AmbientColorArr [3].GetFloat();
	}
}

IDirect3DTexture9* Engine::MaterialInformation::GetTexture(const std::wstring& TexKey) const&
{
	if (auto iter = TextureMap.find(TexKey);
		iter != std::end(TextureMap))
	{
		return iter->second;
	}
	else
	{
		return nullptr;  
	}
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
		}
		Writer.EndObject();

		PropStream << StrBuf.GetString();
	}
}
