#include "RenderObject.h"

void Engine::RenderObject::Initialize(const RenderInterface::Group _Group)&
{
	Super::Initialize();

	RenderInterface::SetUpRenderingInformation(_Group);
};

void Engine::RenderObject::LateUpdate(const float DeltaTime)&
{
	Super::LateUpdate(DeltaTime);
	RenderInterface::Regist();
};

