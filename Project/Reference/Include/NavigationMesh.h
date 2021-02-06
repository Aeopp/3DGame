#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include <vector> 
#include <memory>
#include <optional>
#include <tuple>
#include "MathStruct.h"

namespace Engine
{
	class DLL_DECL NavigationMesh    : public SingletonInterface<NavigationMesh>
	{
	public:
		void InsertPoint(const Vector3 Point)&; 
		void Initialize(IDirect3DDevice9* Device)&;
		void Render(IDirect3DDevice9* const Device)&; 
		std::optional<std::pair<Vector3, uint32>>  
			MoveOnNavigation(const Vector3 TargetLocation,
								const Vector3 TargetDirection ,
								const uint32 CellIndex)&;
	private:
		IDirect3DDevice9* Device{ nullptr }; 
		std::vector<Vector3> CurrentPickPoints{};
		std::vector<Sphere> CurrentMarkers{};
		std::vector<std::shared_ptr<class Cell>> CellContainer{ }; 
		uint32 Index;
	};
};



