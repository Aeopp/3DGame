#include "Collision.h"
#include "Object.h"
#include "Transform.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"


void Engine::Collision::Initialize(
	IDirect3DDevice9* const Device,
	const CollisionTag _Tag,
	class Transform* const OwnerTransform)&
{
	this->Device = Device;
	Super::Initialize();
	this->_Tag = _Tag;
	this->OwnerTransform = OwnerTransform;
	SetUpRenderingInformation(RenderInterface::Group::DebugCollision);
};

void Engine::Collision::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	bCurrentFrameCollision = false;
	this->Owner = Owner;
	CollisionSystem::Instance->Regist(_Tag, this);
	_Geometric->Update(OwnerTransform->GetScale(),
		OwnerTransform->GetRotation(),
		OwnerTransform->GetLocation());
};

void Engine::Collision::Event(Object* Owner)&
{
	RenderInterface::Regist();
};

void Engine::Collision::Render()&
{
	Device->SetTransform(D3DTS_WORLD, &OwnerTransform->UpdateWorld());
	_Geometric->Render(Device, bCurrentFrameCollision);
};

bool Engine::Collision::IsCollision(Collision* const Rhs)&
{
	const auto CollisionInfo = _Geometric->IsCollision(Rhs->_Geometric.get());

	if (CollisionInfo)
	{
		const auto [CrossArea, PushDir] = *CollisionInfo;

		bCurrentFrameCollision = true;
		if (PushCollisionables.contains(Rhs->_Tag))
		{
			Rhs->OwnerTransform->SetLocation(
				Rhs->OwnerTransform->GetLocation() + PushDir *CrossArea);

			Rhs->_Geometric->Update(Rhs->OwnerTransform->GetScale(),
				Rhs->OwnerTransform->GetRotation(),
				Rhs->OwnerTransform->GetLocation());
		}

		Owner->HitNotify(Rhs->Owner, PushDir, CrossArea);
		Rhs->Owner->HitNotify(Owner, -PushDir, CrossArea);
	}

	return CollisionInfo.has_value();
};

