#pragma once
#include "AssimpHelper.h"
#include <vector>

//
//struct Track
//{
//	uint32 PositionKey{ 0u };
//	uint32 RotationKey{ 0u };
//	uint32 ScaleKey{ 0u };
//};
//
//struct Bone
//{
//	int32              ParentIdx{ -1 };
//	int32              TableIdx{ 0 };
//	std::vector<int32> ChildrensIndices{};
//
//	Track              CurrentAnimTrack{};
//	Matrix             FinalMatrix{};
//	Matrix             ToRootSpace{};
//	Matrix             Offset{};
//	Matrix             OriginTransform{};
//	Matrix             CurrentTransform{};
//	std::string        NodeName{};
//};
//
//struct AnimationMesh
//{
//	IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
//	IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
//	IDirect3DTexture9* DiffuseMap{ nullptr };
//	// ��Ű�� ���� ����� ���� ���̺� ���ؽ� ������ �� ������ �Ʒ� ���̺� �迭�� �ε����� ���εǾ������Ƿ�
//	// ������ ������ �ݵ�� ������ ������Ʈ �������.
//	std::vector<Matrix> FinalTransform{};
//	//     ���� ��� �ε����� �� ���� ���̺� �ε��� ����
//	std::unordered_map<uint32, uint32> BoneTableIdxFromFinalTransformIdx{};
//	uint32 NumVertices{ 0u };
//	uint32 FaceCount{ 0u };
//	uint32 MaterialIndex{ 0u };
//	std::vector<uint32> Indices{};
//	std::vector<Vertex::Animation> Vertices;
//};
//
//class MyModel
//{
//public:
//	void Initialize(const std::filesystem::path& Path,
//		const std::filesystem::path& Name,
//		IDirect3DDevice9* const Device)&;
//	void BoneUpdate()&;
//	void Render()&;
//private:
//	const aiNodeAnim* const FindAnimationNode
//	(const aiAnimation* _Animation, const std::string NodeName)&;
//	void BoneUpdateChildren(Bone& Parent, Bone& TargetBone, const aiAnimation* const _Animation, const double t)&;
//	int32 CreateHierarchy(const uint32 ParentIdx, aiNode* const _Node, const Matrix ToRootSpace)&;
//	void CreateMeshInformation(const std::filesystem::path& Path)&;
//	void CreateBuffer(const std::wstring& Name)&;
//private:
//
//	IDirect3DVertexDeclaration9* VertexDecl{ nullptr };
//	std::vector<AnimationMesh> Meshes{};
//	IDirect3DDevice9* Device{};
//	uint32 BoneCount{ 0u };
//	const aiScene* _Scene{};
//	//           ���� �������� ���� ���̺�.
//	std::vector<Bone> BoneTable{};
//	// �˻� ���Ǽ��� ���� �� ���̺�� �� �̸� ����.
//	std::unordered_map<std::string, uint32> BoneTableIndexFromName{};
//};
//
//
struct SkeletonVertex
{
	Vector3 Location{ 0,0,0 };
	Vector3 Normal{ 0 , 0,  0 };
	Vector2 UV{ 0,0 };
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	static inline SkeletonVertex MakeFromAssimpMesh(const aiMesh*const AiMesh ,
													const uint32 CurrentIdx)
	{
		return SkeletonVertex
		{
			FromAssimp(AiMesh->mVertices[CurrentIdx]),  
			FromAssimp(AiMesh->mNormals[CurrentIdx]),
			FMath::ToVec2(FromAssimp(AiMesh->mTextureCoords[0][CurrentIdx]))
		};
	};
};

struct Mesh
{
	IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
	IDirect3DTexture9* DiffuseTexture{ nullptr };
	IDirect3DTexture9* NormalTexture{ nullptr };
	IDirect3DTexture9* SpecularTexture{ nullptr };
	IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
	uint32 FaceCount{ 0u };
	uint32 VtxCount{ 0u };
	uint32 PrimitiveCount{ 0u };
};

class SkeletonMesh
{
public:
	void Load(IDirect3DDevice9*const Device)&;
	void Render()&;
private:
	
	std::vector<Mesh>       MeshContainer{}; 
	IDirect3DDevice9*       Device{ nullptr };
	const aiScene*          AiScene{};
};
