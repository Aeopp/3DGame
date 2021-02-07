#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include <vector> 
#include <filesystem>
#include <memory>
#include <optional>
#include <tuple>
#include "MathStruct.h"

namespace Engine
{
	class DLL_DECL NavigationMesh    : public SingletonInterface<NavigationMesh>
	{
	public:
		void Save(const std::filesystem::path SavePath)const&;
		void Load(const std::filesystem::path SavePath)&;
		void EraseCellFromRay(const Ray WorldRay)&;
		// 광선과 마킹된 마커들중 하나가 충돌할 경우에만 마커의 위치로 삽입.
		bool InsertPointFromMarkers(const Ray WorldRay)&;
		void InsertPoint(const Vector3 Point, const bool bCreateMarker)&;
		void Initialize(IDirect3DDevice9* Device)&;
		void Render(IDirect3DDevice9* const Device)&; 
		std::optional<std::pair<Vector3, uint32>>  
			MoveOnNavigation(const Vector3 TargetLocation,
								const Vector3 TargetDirection ,
								const uint32 CellIndex)&;
		// 파일 로딩시 버텍스버퍼 정보 제대로 채우기.
	public:
		std::vector<Sphere>  CurrentMarkers{};
	private:
		IDirect3DVertexBuffer9* VertexBuffer{ nullptr }; 
		IDirect3DDevice9* Device{ nullptr }; 
		std::vector<Vector3> CurrentPickPoints{};
		std::vector<std::shared_ptr<class Cell>> CellContainer{ }; 
		uint32 Index;
	};
};



