#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <any>
#include <d3d9.h>
#include <unordered_map>
#include <map>
#include "Layer.h"

namespace Engine
{
	class DLL_DECL Scene abstract
	{
	public:
		Scene() = default;
		Scene(Scene&&)noexcept = default;
		virtual ~Scene()noexcept = default;
	public:
		virtual void Initialize(IDirect3DDevice9* const Device)& abstract;
		virtual void Update(const float DeltaTime)&;
		void PendingKill() & noexcept;
	public:
		template<typename LayerSubType,typename...Params>
		auto& NewLayer(Params&&... _Params)&;
		template<typename LayerSubType>
		auto& RefLayer()&;
		auto& RefLayers()&;

		template<typename LayerSubType,typename ObjectSubType>
		auto& FindObject(const std::wstring& TargetName)&;

		template<typename LayerSubType,typename ObjectSubType, typename...Params>
		auto& NewObject(std::wstring ObjectName, Params&&... _Params)&;
	private:
		std::unordered_map<std::string,std::unique_ptr<Layer>> LayerMap;
		IDirect3DDevice9* Device{ nullptr };
	};
};

template<typename LayerSubType, typename ObjectSubType>
inline auto& Engine::Scene::FindObject(const std::wstring& TargetName)&
{
	return RefLayer<LayerSubType>()->FindObject<ObjectSubType>(TargetName);
}
template<typename LayerSubType, typename ObjectSubType, typename ...Params>
inline auto& Engine::Scene::NewObject(
	std::wstring ObjectName, Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	return RefLayer<LayerSubType>()->NewObject<ObjectSubType>
		 		( std::move(ObjectName)  ,std::forward<Params>(_Params)...);
};

inline auto& Engine::Scene::RefLayers()&
{
	return LayerMap;
};

template<typename LayerSubType, typename ...Params>
inline auto& Engine::Scene::NewLayer(Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	auto InsertLayer = std::make_unique<LayerSubType>();
	InsertLayer->Initialize(std::forward<Params>(_Params)...);
	return LayerMap.insert({ typeid(LayerSubType).name(),std::move(InsertLayer) } ).first->second;
}

template<typename LayerSubType>
inline auto& Engine::Scene::RefLayer()&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	return LayerMap.find(typeid(LayerSubType).name())->second;
}

