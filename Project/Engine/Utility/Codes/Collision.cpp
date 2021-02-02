#include "Collision.h"
#include "Object.h"
#include "Transform.h"
#include "Vertexs.hpp"
#include "ResourceSystem.h"
#include "imgui.h"

void Engine::Collision::Initialize(
	IDirect3DDevice9* const Device,
	const CollisionTag _Tag,
	class Transform* const OwnerTransform)&
{
	Super::Initialize();
	this->Device = Device;
	this->_Tag = _Tag;
	this->OwnerTransform = OwnerTransform;
	SetUpRenderingInformation(RenderInterface::Group::DebugCollision);
	RenderInterface::bCullingOn = false;
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
	bImmobility = false;
	CurrentCheckedCollisionIDs.clear();
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

		if (PushCollisionables.contains(Rhs->_Tag) && false==Rhs->bImmobility)
		{
			if (Rhs->PushCollisionables.contains(_Tag) && false==bImmobility)
			{
				Rhs->OwnerTransform->SetLocation(
					Rhs->OwnerTransform->GetLocation() + PushDir * CrossArea*0.5f);

				Rhs->_Geometric->Update(Rhs->OwnerTransform->GetScale(),
										Rhs->OwnerTransform->GetRotation(),
										Rhs->OwnerTransform->GetLocation());

				OwnerTransform->SetLocation(
					OwnerTransform->GetLocation() + -PushDir * CrossArea * 0.5f);

				_Geometric->Update(Rhs->OwnerTransform->GetScale(),
					OwnerTransform->GetRotation(),
					OwnerTransform->GetLocation());
			}
			else
			{
				Rhs->OwnerTransform->SetLocation(
					Rhs->OwnerTransform->GetLocation() + PushDir * CrossArea);

				Rhs->_Geometric->Update(Rhs->OwnerTransform->GetScale(),
					Rhs->OwnerTransform->GetRotation(),
					Rhs->OwnerTransform->GetLocation());
			}
		}
		if (!CurrentCheckedCollisionIDs.contains(Rhs->ID))
		{
			CurrentCheckedCollisionIDs.insert(Rhs->ID);
			Rhs->CurrentCheckedCollisionIDs.insert(ID);

			auto iter = HitCollisionIDs.find(Rhs->ID);

			if (iter == std::end(HitCollisionIDs))
			{
				HitCollisionIDs.insert(iter, Rhs->ID);
				Rhs->HitCollisionIDs.insert(ID);
				Owner->HitBegin(Rhs->Owner, PushDir, CrossArea);
				Rhs->Owner->HitBegin(Owner, -PushDir, CrossArea);
			}
			else
			{
				Owner->HitNotify(Rhs->Owner, PushDir, CrossArea);
				Rhs->Owner->HitNotify(Owner, -PushDir, CrossArea);
			}
		}
	}

	return CollisionInfo.has_value();
};

