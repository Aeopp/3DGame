#include "HeightMap.h"
#include "imgui.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "Transform.h"
#include "Management.h"



void Engine::HeightMap::Initialize()&
{
	Super::Initialize();

	AddComponent<Transform>();
}

void Engine::HeightMap::PrototypeInitialize(
	IDirect3DDevice9* const Device, 
	const RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device, _Group);	

	auto& ResourceSys = ResourceSystem::Instance;
	ResourceSys->Emplace<IDirect3DTexture9>(L"MapTexture",
		D3DXCreateTextureFromFile, Device, L"..\\..\\..\\Resource\\Texture\\Player1.jpg", &Texture);

	Device->CreateVertexBuffer(sizeof(Vertex::Texture) * 3u, D3DUSAGE_DYNAMIC,
		Vertex::Texture::FVF, D3DPOOL_DEFAULT, &VertexBuffer, nullptr);
	ResourceSys->Create<IDirect3DVertexBuffer9>(L"VertexBuffer", VertexBuffer);

	Vertex::Texture* ptr;
	VertexBuffer->Lock(0, sizeof(Vertex::Texture) * 3u, (void**)&ptr, 0);
	ptr[0].Location = {-0.5,-0.5,0.f};
	ptr[0].TexCoord = {0,1};
	ptr[1].Location = { 0.5,0.5,0.f };
	ptr[1].TexCoord = { 1,0 };
	ptr[2].Location = { 0.5,-0.5,0.f };
	ptr[2].TexCoord = { 1,1 };
	VertexBuffer->Unlock();
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
	Device->SetFVF(Vertex::Texture::FVF);
	auto _Transform = GetComponent < Transform>();
	Device->SetTransform(D3DTS_WORLD, &_Transform->UpdateWorld());
	Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Texture));
	Device->SetTexture(0, Texture);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
}
