#pragma once
#include "RenderObject.h"
#include "Model.h"



struct Track
{
	uint32 PositionKey{ 0u };
	uint32 RotationKey{ 0u };
	uint32 ScaleKey{ 0u };
};

struct Bone
{
	int32              ParentIdx{ -1 };
	int32              TableIdx{ 0 };
	std::vector<int32> ChildrensIndices{};

	Track              CurrentAnimTrack{};
	Matrix             FinalMatrix{};
	Matrix             ToRootSpace{};
	Matrix             Offset{};
	Matrix             OriginTransform{};
	Matrix             CurrentTransform{};
	std::string        NodeName{};
};

struct AnimationMesh
{
	IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
	IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
	IDirect3DTexture9* DiffuseMap{ nullptr };
	// ��Ű�� ���� ����� ���� ���̺� ���ؽ� ������ �� ������ �Ʒ� ���̺� �迭�� �ε����� ���εǾ������Ƿ�
	// ������ ������ �ݵ�� ������ ������Ʈ �������.
	std::vector<Matrix> FinalTransform{};
	//     ���� ��� �ε����� �� ���� ���̺� �ε��� ����
	std::unordered_map<uint32, uint32> BoneTableIdxFromFinalTransformIdx{};
	uint32 NumVertices{ 0u };
	uint32 FaceCount{ 0u };
	uint32 MaterialIndex{ 0u };
	std::vector<uint32> Indices{};
	std::vector<Vertex::Animation> Vertices;
};

class MyModel
{
public:
	void Initialize(const std::filesystem::path& Path,
		const std::filesystem::path& Name,
		IDirect3DDevice9* const Device)&;
	void BoneUpdate()&;
	void Render()&;
private:
	const aiNodeAnim* const FindAnimationNode
	(const aiAnimation* _Animation, const std::string NodeName)&;
	void BoneUpdateChildren(Bone& Parent, Bone& TargetBone, const aiAnimation* const _Animation, const double t)&;
	int32 CreateHierarchy(const uint32 ParentIdx, aiNode* const _Node, const Matrix ToRootSpace)&;
	void CreateMeshInformation(const std::filesystem::path& Path)&;
	void CreateBuffer         (const std::wstring& Name)&;
private:
	std::shared_ptr<IDirect3DVertexDeclaration9>VertexDecl;
	std::vector<AnimationMesh> Meshes{};
	IDirect3DDevice9* Device{};
	uint32 BoneCount{ 0u };
	const aiScene* _Scene{};
	//           ���� �������� ���� ���̺�.
	std::vector<Bone> BoneTable{};
	// �˻� ���Ǽ��� ���� �� ���̺�� �� �̸� ����.
	std::unordered_map<std::string, uint32> BoneTableIndexFromName{};
};




class TombStone final: public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& SpawnLocation  ,const Vector3& Rotation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device ,
							const Engine::RenderInterface::Group _Group)&;
	virtual void Event()&override;
	virtual void Render() & override;
	virtual void Update(const float DeltaTime)&;

	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale)&override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;
private:
	MyModel _Model;
	uint32 _TestID = 0u;
	IDirect3DDevice9* Device{ nullptr };
};

