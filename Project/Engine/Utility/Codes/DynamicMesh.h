#pragma once
#include "Mesh.h"
#include <string>
#include "HierarchyLoader.h"
#include "Animation.h"

namespace Engine
{
	class DLL_DECL DynamicMesh : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		static const inline Property TypeProperty = Property::Render;
	public:
		void PrototypeInitialize(IDirect3DDevice9* const Device,
			const std::filesystem::path& Path,
			const std::filesystem::path& Name)&;
		void Initialize()&;
		void Clone()&;
		virtual void Event(class Object* Owner) & override;
		virtual void Render() & override;
	public:
		const Engine::Bone* GetBone(const std::string BoneName)&;
		bool IsAnimationEnd()&;
		void SetAnimationIdx(const uint8 AnimationIndex)&;
		void PlayAnimation(const float DeltaTime)&;
	private:
		void UpdateBone()&;
		void InitBoneRefMatrix()&;
	private:
		Bone* RootFrame{ nullptr };
		HierarchyLoader* Loader{ nullptr };
		Engine::Aniamtion _Animation{};
		std::vector<MeshContainerSub> MeshContainer{};
	};
}
