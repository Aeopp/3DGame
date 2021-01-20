#include "Object.h"

void Engine::Object::Initialize()&
{}
void Engine::Object::PrototypeInitialize()&
{}

void Engine::Object::Clone()&
{
	// �����Ϸ� ����Ʈ ��������� ȣ�� ���Ŀ� �������縦 �����ؾ��ϴ� ���
	/*for (auto& [Key, _CompContainer] : _Components)
	{
		for (auto& [Key,CurrentComp ]: _CompContainer)
		{
			CurrentComp = std::make_shared<Component>(*(CurrentComp.get()));
		}
	}*/
}
void Engine::Object::Event()&
{
	for (auto& [PropertyKey, ComponentContainer] : _Components)
	{
		for (auto& [Key, CurrentComponent] : ComponentContainer)
		{
			CurrentComponent->Event(this);
		}
	};
};

void Engine::Object::Update(const float DeltaTime)&
{

};

void Engine::Object::LateUpdate(const float DeltaTime)&
{
	
}



void Engine::Object::ComponentUpdate(const float DeltaTime)&
{
	for (auto& [PropertyKey, ComponentContainer] : _Components)
	{
		for (auto& [Key,CurrentComponent] : ComponentContainer)
		{
			CurrentComponent->Update(this, DeltaTime);
		}
	};
}
