#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Scene.h"
#include <type_traits>
#include <numeric>
#include <filesystem>


class StartScene final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Event()& override;
	virtual void Update(const float DeltaTime) & override;
private:

};


class MyModel
{
public:
	MyModel(const std::filesystem::path& Path,
			const std::filesystem::path& Name);
private:
	void CreateHierarchy(aiNode* const _Bone)&;
	void CreateTextures()&;
private:
	uint32 BoneCount{ 0u };
	const aiScene* _Scene{};
	std::vector<aiNode*> BoneTableFromIdx{};
	std::unordered_map<std::string,aiNode*> BoneTableFromName{};
};




