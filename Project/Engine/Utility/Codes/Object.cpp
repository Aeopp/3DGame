#include "Object.h"

void Engine::Object::Initialize()&
{
};

void Engine::Object::LateUpdate(const float DeltaTime)&
{
	ComponentUpdate(DeltaTime);
}

void Engine::Object::ComponentUpdate(const float DeltaTime)&
{
	for (auto& [PropertyKey, ComponentContainer] : _Components)
	{
		for (auto& CurrentComponent : ComponentContainer)
		{
			CurrentComponent->Update(this, DeltaTime);
		}
	};
}
