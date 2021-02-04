#pragma once
#include "AssimpHelper.h"
#include <vector>
#include <unordered_map>


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

/// <summary>
using VtxType = SkeletonVertex;
/// </summary>
struct Mesh
{
	IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
	IDirect3DTexture9*      DiffuseTexture{ nullptr };
	IDirect3DTexture9*      NormalTexture{ nullptr };
	IDirect3DTexture9*      SpecularTexture{ nullptr };
	IDirect3DIndexBuffer9*  IndexBuffer{ nullptr };
	std::vector<VtxType>    Verticies{};
	std::vector<std::vector<Matrix>>     Offsets{};
	std::vector<std::vector<float>>      Weights{};
	std::vector<std::vector<Matrix*>>    Finals{};
	uint32 FaceCount{ 0u };
	uint32 VtxCount{ 0u };
	uint32 PrimitiveCount{ 0u };
};

struct Bone
{
	Bone*Parent{};
	std::vector<Bone*> Childrens{};
	Matrix Final{FMath::Identity()};
	Matrix Transform      { FMath::Identity() };
	Matrix OriginTransform{ FMath::Identity() };
	Matrix ToRoot         { FMath::Identity() };
	Matrix Offset         { FMath::Identity() };
	std::string Name      {};
	void BoneMatrixUpdate(
						  const uint32 TargetAnimIdx,
						  const aiScene* const AiScene,
		                  Bone* Parent,
		                  const float T,
		const uint32 ScaleIdx,
		const uint32 RotationIdx,
		const uint32 PositionIdx)&;
};

class SkeletonMesh
{
public:
	void Load(IDirect3DDevice9*const Device)&;
	void Render()&;
	Bone* MakeHierarchy(
		Bone* BoneParent, const aiNode* const AiNode);
	void UpdateTrackIndex(const float DeltaTime)&;
	Bone* RootBone{ nullptr };
	const aiScene* AiScene{};
	uint32 AnimIdx{ 0u };
	uint32 TrackIdx{ 0u };
	std::unordered_map<std::string, std::shared_ptr<Bone>>BoneTable{};
	double T = 0.0f;
private:
	std::vector<Mesh>       MeshContainer{}; 
	IDirect3DDevice9*       Device{ nullptr };
};