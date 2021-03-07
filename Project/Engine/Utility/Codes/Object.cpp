#include "Object.h"
#include "imgui.h"
#include <string>
#include <iostream>
#include "Management.h"
#include "UtilityGlobal.h"


void Engine::Object::Initialize()&
{}
void Engine::Object::PrototypeInitialize()&
{}

void Engine::Object::Clone()&
{
	// 컴파일러 디폴트 복사생성자 호출 이후에 깊은복사를 수행해야하는 경우
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
	if (Engine::Global::bDebugMode)
	{
		std::string _Name;
		_Name.assign(std::begin(Name), std::end(Name));
		ImGui::Separator();
		ImGui::Text(_Name.c_str());


		if (ImGui::SmallButton(("Kill " + _Name).c_str()))
		{
			this->Kill();
		}
	};

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

void Engine::Object::HitBegin(Object* const Target, const Vector3 PushDir, const float CrossAreaScale)&
{
	
}

void Engine::Object::HitEnd(Object* const Target)&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::Text("%s Overlapped end", Target->GetName().c_str());
	}
	
}

void Engine::Object::HitNotify(Object* const Target,
	const Vector3 PushDir, const float CrossAreaScale)&
{

};

