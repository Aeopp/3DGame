#include "Model.h"

void Engine::Model::LoadModel(const std::filesystem::path& Path, const std::filesystem::path& Name)&
{
	Assimp::Importer AssimpImporter{};

	auto ModelScene = AssimpImporter.ReadFile( (Path/Name).string(),
		aiProcess_Triangulate | //�簢�� ���� -> �ﰢ�� ���� ����Ʈ
		aiProcess_JoinIdenticalVertices | // �ߺ� ���� �ϳ��� ��ġ��
		aiProcess_ConvertToLeftHanded | // �޼� ��ǥ�� ��ȯ (�븻�� ź��Ʈ�� ������ �ش�)
		aiProcess_GenNormals | // �� ������ �븻�� ���� ��� �븻 �����Ѵ�. 
		aiProcess_CalcTangentSpace); // �� ������ ź��Ʈ�� ����ź��Ʈ�� ������� ����

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
