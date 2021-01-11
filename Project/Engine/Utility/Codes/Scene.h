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
		explicit Scene(IDirect3DDevice9& _Device);
		virtual ~Scene()noexcept = default;
		Scene(Scene&&)noexcept = default;
	public:
		virtual void Initialize()& abstract;
		void Update(const float DeltaTime)&;
		void PendingKill() & noexcept;
	public:
		template<typename LayerSubType>
		auto& RefLayer()&;
		auto& RefLayers()&;

		template<typename LayerSubType,typename ObjectSubType>
		auto& FindObject(const std::wstring& TargetName)&;
	private:
		// 레이어의 개수가 많아지면 자료구조를 바꾸는것도 고려하길 바람.
		std::vector<std::unique_ptr<Layer>> _Layers;
		std::reference_wrapper<IDirect3DDevice9> _Device;
	};
};

template<typename LayerSubType, typename ObjectSubType>
inline auto& Engine::Scene::FindObject(const std::wstring& TargetName)&
{
	return RefLayer<LayerSubType>()->FindObject<ObjectSubType>(TargetName);
};

inline auto& Engine::Scene::RefLayers()&
{
	return _Layers;
};

template<typename LayerSubType>
inline auto& Engine::Scene::RefLayer()&
{
	static_assert(std::is_base_of_v<Layer, LayerSubType>, __FUNCTION__);

	return std::find_if(std::begin(_Layers), std::end(_Layers), [TargetId = typeid(LayerSubType)](auto& CurrentLayer) {
		return typeid(std::remove_reference_t<decltype(CurrentLayer)>::element_type)
			==
			TargetId;
	})->second;
}

