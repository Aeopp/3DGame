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

	//// ��� ���̾��� ������Ʈ �����͸� �Ӽ��� �ε����� ������ �迭�� ������.
	//// ������Ʈ�� ������Ʈ ������ �����ϱ� ����.
	//for (auto& [Key, CurrentLayer] : _LayerMap)
	//{
	//	CurrentLayer->LateUpdate(DeltaTime);

	//	for (uint32_t Idx = 0u; Idx < Component::Property::End; ++Idx)
	//	{
	//		auto& CurrentPushComponents = CurrentLayer->RefComponents(static_cast <Component::Property>(Idx));
	//		_Components[Idx].insert(std::end(_Components[Idx]),std::begin(CurrentPushComponents),std::end(CurrentPushComponents));
	//	};
	//}

	//// ���� ������Ʈ�� ������ ������ �°� �� �ٽ� �����ؾ���.
	////_Components[Component::Property::Render];

	///// �⺻ Ʈ������ �浹 ���� ������ ������Ʈ ������Ʈ ����.
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





