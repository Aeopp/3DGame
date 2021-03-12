#include "NavigationMesh.h"
#include "Object.h"
#include "Transform.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "imgui.h"
#include "StringHelper.h"
#include "NaviMeshComponent.h"

void Engine::NaviMeshComponent::Initialize()
{
	Super::Initialize();

	NaviMesh = NavigationMesh::Instance.get();

};

void Engine::NaviMeshComponent::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	auto OwnerTransform = Owner->GetComponent<Engine::Transform>();
	const Vector3 OwnerLocation  = OwnerTransform->GetLocation();
};

void Engine::NaviMeshComponent::Event(Object* Owner)&
{
	Super::Event(Owner);

};

