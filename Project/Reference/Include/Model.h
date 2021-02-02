//#pragma once
//#include "TypeAlias.h"
//#include "DllHelper.H"
//#include "Vertexs.hpp"
//#include "FMath.hpp"
//#include <string>
//#include <vector> 
//#include <unordered_map>
//#include <filesystem>
//#include "AssimpHelper.h"
//
////struct Texture
////{
////	uint32 ID;
////	std::string Type;
////	std::string Path;
////};
//
//namespace Engine
//{
//	struct DLL_DECL Bone
//	{
//		Bone() { Offset = FMath::Identity(Offset); };
//		int32 ID = 0;
//		std::string Name = "";
//		// ������ ���.. (��Ȯ�� �ǹ̴� ������)  
//		Matrix Offset;
//		// �ڽĵ�
//		std::vector<Bone> Children{};
//	};
//	struct DLL_DECL BoneTransformTrack
//	{
//		// ���� Ʈ�� �ִϸ��̼� Ÿ�� ��Ŀ
//		std::vector<float> PositionTimeStamps{};
//		std::vector<float> RotationTimeStamps{};
//		std::vector<float> ScalTimeStamps{};
//
//		std::vector<Vector3> Positions{};
//		std::vector<Quaternion> Rotations{};
//		std::vector<Vector3> Scales{};
//
//	};
//	struct DLL_DECL Animation
//	{
//		float Duration = 0.0f;
//		float TicksPerSecond = 1.0f;
//		     // �ش� �ִϸ��̼� �� �̸�  , 
//		     //�ش� �ִϸ��̼� ���� �ش� Ű ������ ������ ȸ��,�̵�,�����ϸ� ��
//		std::unordered_map<std::string, BoneTransformTrack> BoneTransforms{};
//	};
//	class DLL_DECL Model
//	{
//	private:
//	public:
//		static bool ReadSkeleton(Bone& BoneOutput, aiNode* Node,
//			std::unordered_map<std::string, std::pair<int32, Matrix>>&
//			BoneInfoTable);
//		static void LOAD_MODEL(const aiScene* Scene, aiMesh* Mesh,
//			std::vector<Vertex::Animation>& VerticesOutput,
//			std::vector<uint32>& IndicesOutput,
//			Bone& SkeletonOutput, uint32& BoneCount); 
//		static void LoadAnimation(const aiScene* Scene,
//			Animation& _Animation);
//		static std::pair<uint32, float> GetTimeFraction(std::vector<float>&
//			Times, float& DeltaTime); 
//		void GetPose(Animation& _Animation,
//			Bone& Skeleton,
//			float DeltaTime,
//			std::vector<Matrix>& Output,
//			Matrix& ParentTransform,
//			Matrix& GlobalInverseTransform);
//
//		IDirect3DVertexBuffer9* VertexBuffer{};
//		IDirect3DTexture9* _Texture{};
//		
//		void LoadModel(const std::filesystem::path& Path,
//						const std::filesystem::path& Name ,
//			IDirect3DDevice9*const Device)&;	
//		void Render(IDirect3DDevice9* Device)&;
//		/*std::vector<IDirect3DVertexBuffer9*> Vertecies{};
//		IDirect3DVertexBuffer9* VertexBuffer{};
//		uint32 CountAllFacices = 0u;*/
//		
//		/*std::vector<Vertex::Texture > Vertices;
//		std::vector<Index::_16> Indices;
//		std::vector<IDirect3DTexture9*> Textures;
//		void Render()
//		{
//			
//		};*/
//	};
//}
//
//
