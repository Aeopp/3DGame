#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include "Scene.h"

namespace Engine
{
	class DLL_DECL Management : public SingletonInterface<Management>
	{
	public:
		template<typename SceneType>
		void SetScene(IDirect3DDevice9& _Device) & noexcept;
		void Update(const float DeltaTime)&;
	private:
		std::unique_ptr<Scene> _CurrentScene{ nullptr };
	};
};

template<typename SceneType>
inline void Engine::Management::SetScene(IDirect3DDevice9& _Device) & noexcept
{
	static_assert(std::is_base_of_v < Scene, SceneType>,
		"is_base_of_v <Scene,SceneType>");

	_CurrentScene = std::make_unique<SceneType>(_Device);
	_CurrentScene->Initialize();
}
