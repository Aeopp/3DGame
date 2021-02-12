#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include <vector> 
#include <filesystem>
#include <unordered_map>
#include <list>
#include <memory>
#include <optional>
#include <tuple>
#include "MathStruct.h"
#include "Cell.h"
#include <set>
#include <sstream>
#include "ShaderFx.h"
#include "FMath.hpp"

namespace Engine
{
	struct DLL_DECL Marker
	{
		Sphere _Sphere{};
		std::set<uint32> SharedCellKeys{}; 
	};

	class DLL_DECL NavigationMesh    : public SingletonInterface<NavigationMesh>
	{
	public:
		void Clear();
		void CellNeighborLink()&;
		void Save(const std::filesystem::path SavePath,const Matrix& MapWorld)&;
		void Load(const std::filesystem::path LoadPath , const Matrix& MapWorld)&;
		void DebugLog()&;
		void EraseCellFromRay(const Ray WorldRay)&;
		void MarkerMove(const uint32 MarkerKey, const Vector3 Vec)&;
		// ������ ��ŷ�� ��Ŀ���� �ϳ��� �浹�� ��쿡�� ��Ŀ�� ��ġ�� ����.
		uint32 InsertPointFromMarkers(const Ray WorldRay)&;
		uint32 SelectMarkerFromRay(const Ray WorldRay)&;
		uint32 SelectCellFromRay(const Ray WorldRay)&;
		uint32 InsertPoint(const Vector3 Point)&;
		// ��ȯ���� 0 �� ��� ��Ŀ ������ ����.
		void Initialize(IDirect3DDevice9* Device)&;
		void Render(IDirect3DDevice9* const Device)&; 
		std::optional<std::pair<Vector3,const Cell*>>
			MoveOnNavigation(
				const Vector3 TargetDirection,
				const Vector3 TargetLocation,
				const Cell* CurrentPlacedCell)const&;
		// ���� �ε��� ���ؽ����� ���� ����� ä���.
	public:
		std::unordered_map<uint32,std::shared_ptr<Marker>> CurrentMarkers{};
		Vector4 SelectColor{ 1.0f,0.352f,0.307f,1.0f };
		Vector4 NeighborColor{ 0.784f,0.693f,0.784f,0.5f };
		Vector4 DefaultColor{ 0.307f,0.648f,0.920f,0.309f };
	private:
		Matrix World = FMath::Identity();
		Engine::ShaderFx* _ShaderFx { nullptr };
		uint32 CurSelectCellKey{ 0u };
		uint32 CurSelectMarkerKey{ 0u };
		std::string NaviMeshInfoString{};
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		IDirect3DDevice9* Device{ nullptr }; 
						/* Marker Key , Position */
		std::vector<std::pair<uint32,Vector3>>           CurrentPickPoints{};
		std::unordered_map<uint32,std::shared_ptr<Cell>> CellContainer{};
	};
};



