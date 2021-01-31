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

struct Bone
{
	int32 TableIdx{ 0u };
	Matrix ToRootSpace{};
	Matrix Offset{};
	Matrix Transform{};
};

struct MeshInformation
{
	IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
	IDirect3DIndexBuffer9* IndexBuffer{nullptr};
	IDirect3DTexture9* DiffuseMap{ nullptr };
	// ��Ű�� ���� ����� ���� ���̺� ���ؽ� ������ �� ������ �Ʒ� ���̺� �迭�� �ε����� ���εǾ������Ƿ�
	// ������ ������ �ݵ�� ������ ������Ʈ �������.
	std::vector<Matrix> FinalTransform{};
	             //     ���� ��� �ε����� �� ���� ���̺� �ε��� ����
	std::unordered_map<uint32,uint32> BoneTableIdxFromFinalTransformIdx{};
	uint32 MaterialIndex{ 0u };
	std::vector<uint32> Indices{};
};

class MyModel
{
public:
	MyModel(const std::filesystem::path& Path,
			const std::filesystem::path& Name,
			IDirect3DDevice9*const Device);
private:
	void CreateHierarchy(aiNode*const _Node,const Matrix ToRootSpace)&;
	void CreateMeshInformation(const std::filesystem::path& Path)&;
private:
	std::vector<MeshInformation> _MeshInformations{};
	IDirect3DDevice9* Device{};
	uint32 BoneCount{ 0u };
	const aiScene* _Scene{};
	//           ���� �������� ���� ���̺�.
	std::vector<Bone> BoneTable{};
	// �˻� ���Ǽ��� ���� �� ���̺�� �� �̸� ����.
	std::unordered_map<std::string,uint32> BoneTableIndexFromName{};
};




