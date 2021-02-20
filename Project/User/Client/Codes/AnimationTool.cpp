#include "..\\stdafx.h"
#include "AnimationTool.h"
#include "ExportUtility.hpp"
#include "ResourceSystem.h"
#include "PrototypeManager.h"
#include "App.h"
#include "FMath.hpp"
#include "DynamicCamera.h"
#include "Controller.h"
#include "NormalLayer.h"

void AnimationTool::Initialize(IDirect3DDevice9* const Device)&
{
	Super::Initialize(Device);

	auto& ResourceSys = RefResourceSys();
	auto& Proto = RefProto();
	auto& Manager = RefManager();
	auto& Control = RefControl();

	// ���� �� ���̾� �߰�.
	{
		Manager.NewLayer<Engine::NormalLayer>();
	}

	// ������Ÿ�� �ε�.
	{

	}

	// ������Ʈ �߰�.
	{
		constexpr float Aspect = App::ClientSize<float>.first / App::ClientSize<float>.second;

		Manager.NewObject<Engine::NormalLayer, Engine::DynamicCamera>(
			L"Static", L"Camera",
			FMath::PI / 3.f, 0.1f, 10000.f, Aspect, 1000.f, &Control);
	}
}

void AnimationTool::Event()&
{
	Super::Event();

}

void AnimationTool::Update(const float DeltaTime)&
{
	Super::Update(DeltaTime);
}

void AnimationTool::Render()&
{
	Super::Render();
}
