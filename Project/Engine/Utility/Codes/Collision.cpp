#include "Collision.h"
#include "Object.h"
#include "Transform.h"

void Engine::Collision::Initialize(const CollisionTag _Tag,
							class Transform* const OwnerTransform)&
{
	Super::Initialize();
	this->_Tag = _Tag;
	this->OwnerTransform = OwnerTransform;
};

void Engine::Collision::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	CollisionSystem::Instance->Regist(_Tag, this);

	_Geometric->Update( OwnerTransform->GetScale(),
						OwnerTransform->GetRotation(),
						OwnerTransform->GetLocation());
};

void Engine::Collision::Event(Object* Owner)&
{
	
};

bool Engine::Collision::IsCollision(Collision* const Rhs)&
{
	Vector3 PushDir; 
	float CrossAreaScale; 
	const bool bCollision= _Geometric->IsCollision(Rhs->_Geometric.get() ,
						PushDir , CrossAreaScale  );
	if (bCollision)
	{
		if (PushCollisionables.contains(Rhs->_Tag))
		{
			OwnerTransform->SetLocation(
				OwnerTransform->GetLocation() + PushDir
				* CrossAreaScale);

			_Geometric->Update(OwnerTransform->GetScale(),
								OwnerTransform->GetRotation(),
								OwnerTransform->GetLocation());
		}
	}

	return bCollision;
};

