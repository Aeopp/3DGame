#include "Collision.h"

void Engine::Collision::Initialize(const CollisionTag _Tag)&
{
	Super::Initialize();
	this->_Tag = _Tag;
};

void Engine::Collision::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	CollisionSystem::Instance->Regist(_Tag, this);

};

void Engine::Collision::Event(Object* Owner)&
{
	
};

bool Engine::Collision::IsCollision(Collision* const Rhs)&
{
	return _Geometric->IsCollision(Rhs->_Geometric.get());
};



