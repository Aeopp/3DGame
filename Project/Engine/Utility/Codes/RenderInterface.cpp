#include "RenderInterface.h"
#include "Renderer.h"

void Engine::RenderInterface::SetUpRenderingInformation(const Group _Group)
{
	this->_Group = _Group;
}

void Engine::RenderInterface::Regist()
{
	Renderer::Instance->Regist(this);
}
