#include "Component.h"

static uint32 ComponentID = 0;

void Engine::Component::Initialize()&
{
	ID = ComponentID++;

}

void Engine::Component::Update(class Object* const Owner,
	const float DeltaTime)&
{

}

void Engine::Component::Event(Object* const Owner)&
{

}
