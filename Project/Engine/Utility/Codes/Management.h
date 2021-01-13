#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include "Scene.h"
#include "TypeAlias.h"
#include <filesystem>
#include "GraphicDevice.h"
#include "PrototypeManager.h"


namespace Engine
{
	class DLL_DECL Management : public SingletonInterface<Management>
	{

	public:
		void Initialize(const HWND _Hwnd,
			const HINSTANCE HInstance,
			const bool bFullScreen,
			const std::pair<uint32, uint32> ClientSize,
			const D3DMULTISAMPLE_TYPE MultiSample,
			const float DeltaMax,
			const std::filesystem::path& SoundPath)&;
	public:
		~Management()noexcept;
	public:
		void GameLoop()&;
	public:
		template<typename SceneType>
		void SetScene() & noexcept;
		void Event()&;
		void Update(const float DeltaTime)&;
		void Render()&;
		void LastEvent()&;
	public:
		template<typename LayerSubType, typename...Params>
		auto NewLayer(Params&&... _Params)&;
		template<typename LayerSubType>
		auto FindLayer()&;
		auto& RefLayers()&;
	public:
		template<typename LayerSubType, 
			typename ObjectSubType, typename...Params>
		auto NewObject(const std::wstring& PrototypeTag,
							std::wstring ObjectName, 
							Params&&... _Params)&;
		template<typename LayerSubType>
		auto& RefObjects();
		template<typename LayerSubType, typename ObjectSubType>
		auto FindObjects();
		template<typename LayerSubType, typename ObjectSubType>
		auto FindObject(const std::wstring& TargetName)&;
	private:
		HWND Hwnd;
		std::pair<uint32, uint32> ClientSize;
		std::unique_ptr<Scene> _CurrentScene{ nullptr };
	public:
		class PrototypeManager* _PrototypeManager{ nullptr };
		class GraphicDevice* _GraphicDevice{ nullptr };
		class Sound* _Sound{ nullptr };
		class Timer* _Timer{ nullptr };
		class Controller* _Controller{ nullptr };
		class Renderer* _Renderer{ nullptr };
		class ShaderManager* _ShaderManager{ nullptr };
		class FontManager* _FontManager{ nullptr };
	};
};

inline auto& Engine::Management::RefLayers()&
{
	return _CurrentScene->RefLayers();
};


template<typename SceneType>
inline void Engine::Management::SetScene() & noexcept
{
	static_assert(std::is_base_of_v <Scene, SceneType>,
		"is_base_of_v <Scene,SceneType>");

	_CurrentScene = std::make_unique<SceneType>();
	_CurrentScene->Initialize(_GraphicDevice->GetDevice().get());
}


template<typename LayerSubType, typename ...Params>
inline auto Engine::Management::NewLayer(Params && ..._Params)&
{
	return _CurrentScene->NewLayer<LayerSubType>(std::forward<Params>(_Params)...);
}

template<typename LayerSubType, typename ObjectSubType, typename ...Params>
inline auto Engine::Management::NewObject(
	const std::wstring& PrototypeTag,
	std::wstring ObjectName,
	Params && ..._Params)&
{
	auto _Clone=_PrototypeManager->Clone<ObjectSubType>(PrototypeTag);
	_Clone->SetName(std::move(ObjectName));
	_Clone->Initialize(std::forward<Params>(_Params)...);
	return _CurrentScene->NewObject<LayerSubType>(std::move(_Clone));
}

template<typename LayerSubType>
inline auto& Engine::Management::RefObjects()
{
	return FindLayer<LayerSubType>()->RefObjects();
};

template<typename LayerSubType, typename ObjectSubType>
inline auto Engine::Management::FindObjects()
{
	return FindLayer<LayerSubType>()->FindObjects<ObjectSubType>();
};

template<typename LayerSubType, typename ObjectSubType>
inline auto Engine::Management::FindObject(const std::wstring& TargetName)&
{
	return _CurrentScene->FindObject<LayerSubType,ObjectSubType>(TargetName);
};

template<typename LayerSubType>
inline auto Engine::Management::FindLayer()&
{
	return _CurrentScene->FindLayer<LayerSubType>();
};
