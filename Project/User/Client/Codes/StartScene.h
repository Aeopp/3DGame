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
	// 스키닝 계산시 사용할 최종 테이블 버텍스 버퍼의 본 정보는 아래 테이블 배열의 인덱스에 매핑되어있으므로
	// 렌더콜 이전에 반드시 정보를 업데이트 해줘야함.
	std::vector<Matrix> FinalTransform{};
	             //     최종 행렬 인덱스와 본 정보 테이블 인덱스 매핑
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
	//           본의 종합적인 정보 테이블.
	std::vector<Bone> BoneTable{};
	// 검색 편의성을 위한 본 테이블과 본 이름 매핑.
	std::unordered_map<std::string,uint32> BoneTableIndexFromName{};
};




