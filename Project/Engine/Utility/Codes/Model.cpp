#include "Model.h"

void Engine::Model::LoadModel(const std::filesystem::path& Path, const std::filesystem::path& Name)&
{
	Assimp::Importer AssimpImporter{};

	auto ModelScene = AssimpImporter.ReadFile( (Path/Name).string(),
		aiProcess_Triangulate | //사각형 정점 -> 삼각형 정점 컨버트
		aiProcess_JoinIdenticalVertices | // 중복 정점 하나로 합치기
		aiProcess_ConvertToLeftHanded | // 왼손 좌표계 변환 (노말과 탄젠트에 영향을 준다)
		aiProcess_GenNormals | // 모델 정보에 노말이 없을 경우 노말 생성한다. 
		aiProcess_CalcTangentSpace); // 모델 정보에 탄젠트와 바이탄젠트가 없을경우 생성

	for (uint32 MeshIdx = 0u; MeshIdx < ModelScene->mNumMeshes; ++MeshIdx)
	{
		auto& CurretMesh = ModelScene->mMeshes[MeshIdx];
		const uint32 NumVertices = ModelScene->mMeshes[MeshIdx]->mNumVertices;

		for (uint32 VertexIdx = 0u; VertexIdx < NumVertices; ++VertexIdx)
		{
			const uint32 Face = CurretMesh[VertexIdx].mNumFaces;
				const uint32 VerteicesNumber = CurretMesh[VertexIdx].mNumVertices;

				for (uint32 VertexIdx = 0u ; VertexIdx < VerteicesNumber; VertexIdx)
				{
					auto& Vertices = CurretMesh[VertexIdx].mVertices[VertexIdx];

					Vector3  CurrentVertexLocation 
					{
						Vertices.x, 
						Vertices.y ,
						Vertices.z
					};
				}

			Face;
		}
	}
	
}
