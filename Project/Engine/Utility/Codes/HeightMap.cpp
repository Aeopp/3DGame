#include "HeightMap.h"
#include "imgui.h"


void Engine::HeightMap::Initialize(const RenderInterface::Group _Group)&
{
	Super::Initialize(_Group);
}

void Engine::HeightMap::Event() & 
{
	Super::Event();

	ImGui::Begin("TEST");
	static int TestFloat = 1;
	ImGui::SliderInt("FLOAT", &TestFloat, 0.f, 100.f);
	ImGui::End();
}
void Engine::HeightMap::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}

void Engine::HeightMap::Render()&
{
}
