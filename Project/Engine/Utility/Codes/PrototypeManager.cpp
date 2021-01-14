#include "PrototypeManager.h"

void Engine::PrototypeManager::Initialize()
{
}

void Engine::PrototypeManager::Clear() & noexcept
{
	_Container.clear();
}

void Engine::PrototypeManager::Clear(const std::wstring& Tag)
{
	_Container[Tag].clear();
}
