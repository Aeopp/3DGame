#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include "fmod.hpp"
#include <unordered_map>
#include <functional>
#include <filesystem>
#include "TypeAlias.h"
namespace Engine
{
	class DLL_DECL Sound :public SingletonInterface<Sound>
	{
	public:
		static inline auto FmodDeleter = [](auto  Target) 
		{
			Target->release();
		};
		using SoundType = std::pair<
			std::unique_ptr<FMOD::Sound,decltype(FmodDeleter)>, FMOD::Channel*>;

		std::unique_ptr<FMOD::System, decltype(FmodDeleter)> FmodSystem{  };
	public:
		void Initialize(const std::filesystem::path& FilePath)&;
		// 확장자없이 파일이름만 입력해주세요.
		void Play(
			const std::string & SoundKey, 
			const float Volume, 
			const bool bBeginIfPlaying,/*재생중일경우 처음부터 다시 재생할지 여부*/
			 const bool IsBgm = false)&;
		void Stop(const std::string& SoundKey)&;
		void Load(const std::string& FullPath, std::string Key)&;
		void Update(const float DeltaTime)&;
		bool IsPlay(const std::string& SoundKey)const&;
		void RandSoundKeyPlay(const std::string& SoundKey,
			const std::pair<uint32, uint32> Range, const float Volume)&;
	private:
		std::unordered_map<std::string, SoundType> Sounds;
		std::string CurrentBgmKey{};
	};

	void SoundPlay(const std::string& Key, 
		const float Volume = 1.f, 
		const bool IsBgm = false);

	void RandSoundPlay(const std::string& Key, 
		const std::pair<uint32, uint32> Range,
		const float Volume = 1.f, 
		const bool IsBgm = false);
};

