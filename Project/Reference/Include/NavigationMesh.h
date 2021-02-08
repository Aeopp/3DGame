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
		void CellNeighborLink()&;
		void Save(const std::filesystem::path SavePath)&;
		void Load(const std::filesystem::path LoadPath)&;
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
	private:
		uint32 CurSelectCellKey{ 0u };
		uint32 CurSelectMarkerKey{ 0u };
		std::string LastSaveFileBuffer{};
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
		IDirect3DDevice9* Device{ nullptr }; 
						/* Marker Key , Position */
		std::vector<std::pair<uint32,Vector3>>           CurrentPickPoints{};
		std::unordered_map<uint32,std::shared_ptr<Cell>> CellContainer{};
	};
};



