#include "AnimEffect.h"
#include "ResourceSystem.h"
#include "Renderer.h"
#include "imgui.h"
#include "EffectSystem.h"
#include <filesystem>

void Engine::EffectSystem::Initialize(IDirect3DDevice9* const Device)&
{
	std::filesystem::directory_iterator itr(Engine::Global::ResourcePath / L"Effect" / L"tex");
	while (itr != std::filesystem::end(itr)) {
		const std::filesystem::directory_entry& entry = *itr;
		IDirect3DTexture9* TextureTemp{ nullptr }; 
		const auto& Extension =entry.path().extension();
		if (Extension == L"tga" || Extension == L"png")
		{
			D3DXCreateTextureFromFile(Device, entry.path().c_str(), &TextureTemp);

			if (TextureTemp)
			{
				const std::wstring TexResourceKey = entry.path().filename().stem();
				ResourceSystem::Instance->Insert<IDirect3DTexture9>(TexResourceKey, TextureTemp);
				EffectTextures[TexResourceKey] = TextureTemp;
			}
		}
		
		itr++;
	}



	int i = 0;


	


};

void Engine::EffectSystem::LoadEffect(IDirect3DDevice9* const Device, const std::filesystem::path FileFullPath, const std::wstring& ResourceName, const EffectType _EffectType)&
{
	if (_EffectType == EffectType::AnimEffect)
	{
		std::shared_ptr<AnimEffect> _AnimEft = std::make_shared<AnimEffect>();
		_AnimEft->Load(Device, FileFullPath);
		ProtoAnimEffects.insert({ ResourceName  ,_AnimEft  });
	}
}

Engine::AnimEffect*  Engine::EffectSystem::MakeEffect(const std::wstring& ResourceName, const EffectType _EffectType)&
{
	if (_EffectType == EffectType::AnimEffect)
	{
		auto _CurMakeEft = std::make_shared<AnimEffect>(AnimEffect{ *ProtoAnimEffects.find(ResourceName)->second });
		_CurMakeEft->Initialize();
		AnimEffects.push_back(_CurMakeEft);
		return _CurMakeEft.get();
	};

	return nullptr;
}

void Engine::EffectSystem::Update(const float DeltaTime)&
{
	for (auto& _CurAnimEffect : AnimEffects)
	{
		_CurAnimEffect->Update(DeltaTime);
	}
}

void Engine::EffectSystem::RenderReady(Renderer* const _Renderer)&
{
	for (auto& _CurAnimEffect: AnimEffects)
	{
		_CurAnimEffect->RenderReady(_Renderer);
	}
}

void Engine::EffectSystem::Render(Renderer* const _Renderer)&
{
	for (auto& _CurAnimEffect : AnimEffects)
	{
		_CurAnimEffect->Render(_Renderer);
	}
}

