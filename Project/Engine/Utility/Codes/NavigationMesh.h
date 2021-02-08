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

namespace Engine
{
	struct DLL_DECL Marker
	{
		Sphere _Sphere{};
		std::vector<uint32> SharedCellKeys{}; 
	};

	class DLL_DECL NavigationMesh    : public SingletonInterface<NavigationMesh>
	{
	public:
		void CellNeighborLink()&;
		void Save(const std::filesystem::path SavePath)const&;
		void Load(const std::filesystem::path SavePath)&;
		void EraseCellFromRay(const Ray WorldRay)&;
		// ������ ��ŷ�� ��Ŀ���� �ϳ��� �浹�� ��쿡�� ��Ŀ�� ��ġ�� ����.
		bool InsertPointFromMarkers(const Ray WorldRay)&;
		void InsertPoint(const Vector3 Point)&;
		void Initialize(IDirect3DDevice9* Device)&;
		void Render(IDirect3DDevice9* const Device)&; 
		std::optional<std::pair<Vector3,  const Cell*>>
			MoveOnNavigation(
				const Vector3 TargetDirection,
				const Vector3 TargetLocation,
				const Cell* CurrentPlacedCell)const&;
		// ���� �ε��� ���ؽ����� ���� ����� ä���.
	public:
		std::unordered_map<uint32,std::shared_ptr<Marker>> CurrentMarkers{};
	private:
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
		IDirect3DDevice9* Device{ nullptr }; 
						/* Marker Key , Position */
		std::vector<std::pair<uint32,Vector3>> CurrentPickPoints{};
		std::unordered_map<uint32,std::shared_ptr<Cell> > CellContainer{};
	};
};



