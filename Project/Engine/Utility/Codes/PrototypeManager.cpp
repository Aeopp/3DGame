#include "PrototypeManager.h"
#include "imgui.h"
#include "StringHelper.h"
#include "UtilityGlobal.h"

void Engine::PrototypeManager::Initialize()
{
}

void Engine::PrototypeManager::Clear() & noexcept
{
	_Container.clear();
}

void Engine::PrototypeManager::Clear(const std::wstring& Tag)
{
	_Container[Tag].clear();
}

void Engine::PrototypeManager::ClearExceptTag(const std::wstring& Tag)&
{
	std::erase_if(
		_Container, [Tag](const auto& Key_InnerContainer)
		{
			return Key_InnerContainer.first != Tag;
		});
}

void Engine::PrototypeManager::Event()&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Class List");
		for (const auto& [LayerTag, InnerContainer] : _Container)
		{
			if (ImGui::TreeNode(ToA(LayerTag).c_str()))
			{
				for (const auto& [ProtoTag, ObjectPtr] : InnerContainer)
				{
					if (ImGui::TreeNode((ProtoTag).c_str()))
					{
						ObjectPtr->PrototypeEdit();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}	
}
