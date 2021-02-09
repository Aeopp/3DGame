#include "..\\stdafx.h"
#include "TestTerrain.h"
#include "Transform.h"
#include "StaticMesh.h"
#include <iostream>
#include "StaticMesh.h"
#include "Collision.h"
#include "CollisionSystem.h"
#include "Controller.h"
#include "ExportUtility.hpp"
#include "dinput.h"
#include "imgui.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "App.h"
#include "ShaderManager.h"

void TestTerrain::Initialize(
	const Vector3& Scale,
	const Vector3& Rotation,
	const Vector3& SpawnLocation)&
{
	Super::Initialize();

	auto _Transform =AddComponent<Engine::Transform>();
	_Transform->SetScale({ 1,1,1 });
	_Transform->SetScale(Scale);
	_Transform->SetRotation(Rotation);
	_Transform->SetLocation(SpawnLocation);

	auto _StaticMesh =AddComponent<Engine::StaticMesh>(L"TestTerrain");

	RenderInterface::bCullingOn = false;

	TestTerrainFx = Engine::ShaderFx::Load(Device, App::ResourcePath / L"Shader" / "TestTerrain.hlsl", L"ShaderFx_TestTerrainFx");
}

void TestTerrain::PrototypeInitialize(IDirect3DDevice9* const Device,
						const Engine::RenderInterface::Group _Group)&
{
	Super::PrototypeInitialize(Device,_Group);
	this->Device = Device;

	auto _StaticMeshProto = std::make_shared<Engine::StaticMesh>();

	_StaticMeshProto->Load<Vertex::LocationNormal>(Device,
		App::ResourcePath / L"Mesh" / L"StaticMesh" / L"Map" / L"",
		L"terrain2.fbx", L"TestTerrain");

	RefResourceSys().InsertAny<decltype(_StaticMeshProto)>(L"TestTerrain", _StaticMeshProto);
}

void TestTerrain::Event()&
{
	Super::Event();
	ImGui::Begin("TestTerrain");
	ImGui::DragFloat4("Location", (float*)&Sun, 1.f, -1000.f, 1000.f);
	ImGui::VSliderFloat("Intensity", ImVec2{ 50,50 }, &SunPower, -1000.f, +1000.f);
	ImGui::End();
}

void TestTerrain::Render()&
{
	Super::Render();
	auto _StaticMesh = GetComponent<Engine::StaticMesh>();

	Matrix Projection, View;
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	Device->GetTransform(D3DTS_VIEW, &View);
	ID3DXEffect* Fx = TestTerrainFx->GetHandle();
	const Matrix& World = GetComponent<Engine::Transform>()->UpdateWorld();
	Fx->SetFloatArray("Sun", Sun, 3u);
	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetFloat("Power", SunPower);

	uint32 iPassMax = 0;
	Fx->Begin(&iPassMax, 0);
	Fx->BeginPass(0);
	for (auto& CurrentRenderMesh : _StaticMesh->MeshContainer)
	{
		Device->SetFVF(CurrentRenderMesh.FVF);
		Fx->SetTexture("Diffuse", CurrentRenderMesh.DiffuseTexture);
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, CurrentRenderMesh.Stride);
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);

		Device->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 0u, 0u, CurrentRenderMesh.VtxCount, 0u, CurrentRenderMesh.PrimitiveCount);
	}
	Fx->EndPass();
	Fx->End();
}

void TestTerrain::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
};
