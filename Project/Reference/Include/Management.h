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
			const float DeltaMax,
			const std::filesystem::path& ResourcePath)&;
	public:
		~Management()noexcept;
	public:
		void GameLoop()&;
	public:
		template<typename SceneType>
		void ChangeScene() & noexcept;
		// 프레임당 반드시 1번만 호출되는 이벤트
		// 수학,물리적인 계산이 아니라면 이벤트에서 로직을 구성하는게 좋습니다.
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


		
		Engine::Object*  NewObject(
			const std::string& LayerSubTypeName,
			const std::string& ObjectSubTypeName,
			const std::wstring& PrototypeTag,
			std::wstring ObjectName,
			const Engine::Object::SpawnParam& ObjectSpawnParam)&;

		template<typename LayerSubType>
		auto& RefObjects();
		template<typename LayerSubType, typename ObjectSubType>
		auto FindObjects();
		template<typename LayerSubType, typename ObjectSubType>
		auto FindObject(const std::wstring& TargetName)&;
	private:
		void CreateStaticResource()&;
		void CreateCollisionDebugResource()&;
	private:
		IDirect3DDevice9* Device{ nullptr }; 
		HWND Hwnd{ NULL };
		std::pair<uint32, uint32> ClientSize{};
		std::unique_ptr<Scene> _CurrentScene{ nullptr };
		std::function<void()> SceneChangeEvent{};
	public:
		std::filesystem::path ResourcePath{};
		class PrototypeManager* _PrototypeManager{ nullptr };
		class GraphicDevice* _GraphicDevice{ nullptr };
		class Sound* _Sound{ nullptr };
		class Timer* _Timer{ nullptr };
		class Controller* _Controller{ nullptr };
		class Renderer* _Renderer{ nullptr };
		class ShaderManager* _ShaderManager{ nullptr };
		class FontManager* _FontManager{ nullptr };
		class ResourceSystem* _ResourceSys{ nullptr };
		class CollisionSystem* _CollisionSys{ nullptr };
		class NavigationMesh* _NaviMesh{ nullptr }; 
	};
};

inline auto& Engine::Management::RefLayers()&
{
	return _CurrentScene->RefLayers();
}



template<typename SceneType>
inline void Engine::Management::ChangeScene() & noexcept
{
	static_assert(std::is_base_of_v <Scene, SceneType>,
		"is_base_of_v <Scene,SceneType>");

	if (!_CurrentScene)
	{
		_CurrentScene = std::make_unique<SceneType>();
		_CurrentScene->Initialize(_GraphicDevice->GetDevice().get());
	}
	else
	{
		SceneChangeEvent = [this]()
		{
			_PrototypeManager->ClearExceptTag(L"Static");
			_CurrentScene = std::make_unique<SceneType>();
			_CurrentScene->Initialize(_GraphicDevice->GetDevice().get());
		};
	}
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

inline Engine::Object* Engine::Management::NewObject
(const std::string& LayerSubTypeName, const std::string& ObjectSubTypeName, const std::wstring& PrototypeTag, std::wstring ObjectName, 
	const Engine::Object::SpawnParam& ObjectSpawnParam)&
{
	std::shared_ptr<Engine::Object> ClonePtr = _PrototypeManager->Clone(ObjectSubTypeName, PrototypeTag);

	Engine::Object* CloneRawPtr{ nullptr }; 
	
	if (ClonePtr)
	{
		ClonePtr->SetName(std::move(ObjectName));
		auto IsSpawnRV = ClonePtr->InitializeFromEditSpawnParam(ObjectSpawnParam);
		if (IsSpawnRV)
		{
			CloneRawPtr = _CurrentScene->NewObject(LayerSubTypeName, ObjectSubTypeName, ClonePtr);
		}
		
	}
	
	return CloneRawPtr;
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
