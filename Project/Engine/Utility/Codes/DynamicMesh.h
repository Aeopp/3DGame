#pragma once
#include "Mesh.h"
#include <string>
#include "HierarchyLoader.h"

namespace Engine
{
	class DLL_DECL DynamicMesh : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		static const inline Property TypeProperty = Property::Render;
	public:
		void Initialize(IDirect3DDevice9* const Device,
			const std::wstring& MeshResourceName)&;
		virtual void Event(class Object* Owner) & override;
		virtual void Render() & override;
	public:
		const Bone* GetBone(const std::string BoneName)&;
		bool IsAnimationEnd()&;
		void SetAnimationIdx(const uint8 AnimationIndex)&;
		void PlayAnimation(const float DeltaTime)&;
	private:
		D3DXFRAME* RootFrame{ nullptr };
		HierarchyLoader* Loader{ nullptr };
		std::vector<MeshContainerSub> MeshContainer{};
	};
}
