#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include "Scene.h"
#include "TypeAlias.h"
#include <filesystem>


namespace Engine
{
	class DLL_DECL Management : public SingletonInterface<Management>
	{
	public:
		void Initialize(const HWND _Hwnd,
			const bool bFullScreen,
			const std::pair<uint32, uint32> ClientSize,
			const D3DMULTISAMPLE_TYPE MultiSample,
			const uint32 LimitFrame,
			const uint32 LimitDeltaMilliSec ,
			const std::filesystem::path& SoundPath)&;
		void GameLoop()&;
	public:
		template<typename SceneType>
		void SetScene(IDirect3DDevice9& _Device) & noexcept;
		void BeforeUpdateEvent()&;
		void Update(const float DeltaTime)&;
		void Render()&;
		void LastEvent()&;
		template<typename LayerSubType>
		auto& RefObjects();
		template<typename LayerSubType, typename ObjectSubType>
		auto& RefObjects();
		template<typename LayerSubType>
		auto& RefLayer()&;
		std::vector<std::shared_ptr<Layer>>& RefLayers()&;
	private:
		std::pair<uint32, uint32> ClientSize;
		HWND Hwnd;
		std::unique_ptr<Scene> _CurrentScene{ nullptr };
	public:
		class Sound* _Sound{ nullptr };
		class Timer* _Timer{ nullptr };
		class Controller* _Controller{ nullptr };
		class GraphicDevice* _GraphicDevice{ nullptr };
	};
};

template<typename SceneType>
inline void Engine::Management::SetScene(IDirect3DDevice9& _Device) & noexcept
{
	static_assert(std::is_base_of_v <Scene, SceneType>,
		"is_base_of_v <Scene,SceneType>");

	_CurrentScene = std::make_unique<SceneType>(_Device);
	_CurrentScene->Initialize();
}

template<typename LayerSubType>
inline auto& Engine::Management::RefObjects()
{
	return _CurrentScene->RefLayer<LayerSubType>()->RefObjects();
};

template<typename LayerSubType, typename ObjectSubType>
inline auto& Engine::Management::RefObjects()
{
	return _CurrentScene->RefLayer<LayerSubType>()->RefObjects<ObjectSubType>();
};

template<typename LayerSubType>
inline auto& Engine::Management::RefLayer()&
{
	return _CurrentScene->RefLayer<LayerSubType>();
};
