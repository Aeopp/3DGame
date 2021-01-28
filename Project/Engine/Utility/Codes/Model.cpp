#include "Model.h"
#include "ResourceSystem.h"

void Engine::Model::LoadModel(const std::filesystem::path& Path, const std::filesystem::path& Name ,
	IDirect3DDevice9* const Device)&
{
	Assimp::Importer AssimpImporter{};

	auto ModelScene = AssimpImporter.ReadFile( (Path/Name).string(),
		aiProcess_Triangulate | //�簢�� ���� -> �ﰢ�� ���� ����Ʈ
		aiProcess_JoinIdenticalVertices | // �ߺ� ���� �ϳ��� ��ġ��
		aiProcess_ConvertToLeftHanded | // �޼� ��ǥ�� ��ȯ (�븻�� ź��Ʈ�� ������ �ش�)
		aiProcess_GenNormals | // �� ������ �븻�� ���� ��� �븻 �����Ѵ�. 
		aiProcess_CalcTangentSpace); // �� ������ ź��Ʈ�� ����ź��Ʈ�� ������� ����

	std::vector<Vertex::Texture> Vertices;
	uint32 CountAllVertices = 0u;
	
	for (uint32 MeshIdx = 0u; MeshIdx < ModelScene->mNumMeshes; ++MeshIdx)
	{
		auto& CurretMesh = ModelScene->mMeshes[MeshIdx];
		const uint32 NumVertices = CurretMesh->mNumVertices;
		CountAllFacices +=CurretMesh->mNumFaces;
		for(uint32 VertexIdx = 0u; VertexIdx < NumVertices; ++VertexIdx)
		{
			Vertex::Texture CurrentVertex;
			Vector3 Location, Normal; Vector2 UV;

			std::memcpy(&Location,&CurretMesh->mVertices[VertexIdx],
				sizeof(Vector3));
			std::memcpy(&Normal, &CurretMesh->mNormals[VertexIdx],
				sizeof(Vector3));
			std::memcpy(&UV, &CurretMesh->mTextureCoords[VertexIdx],
				sizeof(Vector2));

			Vertices.push_back(Vertex::Texture
				{
					std::move(Location), 
					std::move(Normal),
					std::move(UV) 
				} );
		}
		CountAllVertices += NumVertices;
	}
	
	Device->CreateVertexBuffer(sizeof(Vertex::Texture) * CountAllVertices,
		D3DUSAGE_DYNAMIC, Vertex::Texture::FVF, D3DPOOL_DEFAULT, &VertexBuffer,
		nullptr);
	auto& ResourceSys = ResourceSystem::Instance;
	;
	ResourceSys->Insert<IDirect3DVertexBuffer9>(L"VertexBuffer" + Name.generic_wstring(), VertexBuffer);

	decltype(Vertices)::value_type*  VertexBufferPtr{};
	VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBuffer), NULL);

	std::memcpy(VertexBufferPtr, Vertices.data(),
	sizeof( decltype(Vertices)::value_type )* Vertices.size());

	VertexBuffer->Unlock();
}

void Engine::Model::Render(IDirect3DDevice9* Device)&
{
	Device->SetFVF(Vertex::Texture::FVF);
	Device->SetStreamSource(0, VertexBuffer, 0, sizeof(Vertex::Texture));
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0u, CountAllFacices);
	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);
}

