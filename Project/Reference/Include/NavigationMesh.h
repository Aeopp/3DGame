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
		// 광선과 마킹된 마커들중 하나가 충돌할 경우에만 마커의 위치로 삽입.
		bool InsertPointFromMarkers(const Ray WorldRay)&;
		void InsertPoint(const Vector3 Point)&;
		void Initialize(IDirect3DDevice9* Device)&;
		void Render(IDirect3DDevice9* const Device)&; 
		std::optional<std::pair<Vector3,  const Cell*>>
			MoveOnNavigation(
				const Vector3 TargetDirection,
				const Vector3 TargetLocation,
				const Cell* CurrentPlacedCell)const&;
		// 파일 로딩시 버텍스버퍼 정보 제대로 채우기.
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



