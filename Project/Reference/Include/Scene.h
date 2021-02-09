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
		virtual void Event()&;
		virtual void Update(const float DeltaTime)&;
		virtual void Render()&;
		void PendingKill() & noexcept;
	public:
		template<typename LayerSubType,typename...Params>
		auto NewLayer(Params&&... _Params)&;
		template<typename LayerSubType>
		auto FindLayer()&;
		auto& RefLayers()&;

		template<typename LayerSubType,typename ObjectSubType>
		auto FindObject(const std::wstring& TargetName)&;

		template<typename LayerSubType,typename ObjectSubType>
		auto NewObject(std::shared_ptr<ObjectSubType> PassClone)&;
							
	protected:
		std::unordered_map<std::string,std::unique_ptr<Layer>> LayerMap;
		IDirect3DDevice9* Device{ nullptr };
	};
};

template<typename LayerSubType, typename ObjectSubType>
inline auto Engine::Scene::FindObject(const std::wstring& TargetName)&
{
	auto* const TargetLayer = FindLayer<LayerSubType>();
	ObjectSubType* TargetObject{ nullptr };
	if (TargetLayer == nullptr)
		return TargetObject;

	return TargetLayer->FindObject<ObjectSubType>(TargetName);
}
template<typename LayerSubType, typename ObjectSubType>
inline auto Engine::Scene::NewObject(
	std::shared_ptr<ObjectSubType> PassClone)&
{
	static_assert(std::is_base_of_v<Layer,LayerSubType>, __FUNCTION__);

	return FindLayer<LayerSubType>()->NewObject( std::move(PassClone));
};

inline auto& Engine::Scene::RefLayers()&
{
	return LayerMap;
};

template<typename LayerSubType, typename ...Params>
inline auto Engine::Scene::NewLayer(Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	auto InsertLayer = std::make_unique<LayerSubType>();
	InsertLayer->Initialize(std::forward<Params>(_Params)...);
	return static_cast<LayerSubType* const>
			(LayerMap.insert({ typeid(LayerSubType).name(),std::move(InsertLayer) }).first->second.get());
}

template<typename LayerSubType>
inline auto Engine::Scene::FindLayer()&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	LayerSubType*  TargetLayer{ nullptr };

	auto iter= LayerMap.find(typeid(LayerSubType).name());

	if (iter != std::end(LayerMap))
	{
		TargetLayer = static_cast<LayerSubType* const>(iter->second.get());
	}

	return TargetLayer; 
}

