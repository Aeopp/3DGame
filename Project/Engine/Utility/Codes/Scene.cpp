#include "Scene.h"
#include <array>
#include "Component.h"
#include <iterator>

Engine::Scene::Scene(IDirect3DDevice9& _Device):_Device{ _Device }{}

void Engine::Scene::Update(const float DeltaTime)&
{
	for (auto& CurrentLayer : _Layers)
	{
		CurrentLayer->Update(DeltaTime);
	}

	//std::array<std::vector<std::weak_ptr<Component>>, Component::Property::End> _Components;

	//// 모든 레이어의 컴포넌트 포인터를 속성과 인덱스를 매핑한 배열에 수집함.
	//// 컴포넌트의 업데이트 순서를 통제하기 위함.
	//for (auto& [Key, CurrentLayer] : _LayerMap)
	//{
	//	CurrentLayer->LateUpdate(DeltaTime);

	//	for (uint32_t Idx = 0u; Idx < Component::Property::End; ++Idx)
	//	{
	//		auto& CurrentPushComponents = CurrentLayer->RefComponents(static_cast <Component::Property>(Idx));
	//		_Components[Idx].insert(std::end(_Components[Idx]),std::begin(CurrentPushComponents),std::end(CurrentPushComponents));
	//	};
	//}

	//// 렌더 컴포넌트는 렌더링 순서에 맞게 또 다시 정렬해야함.
	////_Components[Component::Property::Render];

	///// 기본 트랜스폼 충돌 렌더 순으로 컴포넌트 업데이트 수행.
	//for (auto& CurrentComponents : _Components)
	//{
	//	for (auto& CurrentComponent : CurrentComponents)
	//	{
	//		auto CurrentCompShared = CurrentComponent.lock();

	//		if (CurrentCompShared)
	//		{
	//			CurrentCompShared->Update(DeltaTime);
	//		}
	//	}
	//}

}
void Engine::Scene::PendingKill() & noexcept
{
	for (auto& CurrentLayer : _Layers)
	{
		CurrentLayer->PendingKill();
	}
}





