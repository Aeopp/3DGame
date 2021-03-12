#pragma once
#include "Component.h"
#include <set>
#include "NavigationMesh.h"

namespace Engine
{
	class DLL_DECL NaviMeshComponent : public Component
	{
	public:
		struct Information
		{

		};

		using Super = Component;
		void Initialize();
		virtual void Update(class Object* const Owner,
			const float DeltaTime) & override;
		virtual void Event(class Object* Owner) & override;
	private:
		const NavigationMesh* NaviMesh=nullptr;
	};
};
