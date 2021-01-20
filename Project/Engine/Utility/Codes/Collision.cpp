#include "Collision.h"

void Engine::Collision::Initialize(const CollisionTag _Tag)&
{
	Super::Initialize();
	this->_Tag = _Tag;
};

void Engine::Collision::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);

};

bool Engine::Collision::IsCollision(Collision* const Rhs)&
{

};



